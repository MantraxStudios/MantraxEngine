#pragma once
#include "libs/MMaths.hpp"
#include "MBody.h"
#include <iostream>
#include <vector>


class MWorld
{
public:
	std::vector<MBody*> Bodys = std::vector<MBody*>();
	Vector3 GravityDirection = Vector3(0.0f, -1.0f, 0.0f);
	
    float WorldGravity = 9.81f;
    float SLEEP_THRESHOLD = 0.05f;
	
    MBody* CreateBody();

    bool RemoveBody(MBody* body);
    void UpdateWorld(float deltaTime);

    static void ResolveCollision(MBody* a, MBody* b) {
        if (a->BodyType == bDynamic && b->BodyType == bStatic) {
            // Usar bounding box rotado
            Vector3 aRotatedSize = MMaths::GetRotatedBoundingBox(a->AttachShaped->Size, a->Rotation);

            Vector3 aMin = a->Position - aRotatedSize;
            Vector3 aMax = a->Position + aRotatedSize;
            Vector3 bMin = b->Position - b->AttachShaped->Size;
            Vector3 bMax = b->Position + b->AttachShaped->Size;

            // Verificar colisión
            if (aMax.x < bMin.x || aMin.x > bMax.x ||
                aMax.y < bMin.y || aMin.y > bMax.y ||
                aMax.z < bMin.z || aMin.z > bMax.z) {
                return;
            }

            // Calcular overlaps
            float overlapX = std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x);
            float overlapY = std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y);
            float overlapZ = std::min(aMax.z, bMax.z) - std::max(aMin.z, bMin.z);

            // Solo procesar si hay overlap significativo (aumentado el threshold)
            const float minOverlap = 0.05f; // Aumentado para evitar micro-colisiones
            if (overlapX < minOverlap || overlapY < minOverlap || overlapZ < minOverlap) {
                return;
            }

            // Encontrar eje de menor penetración
            float minPenetration = overlapX;
            char axis = 'x';
            if (overlapY < minPenetration) { minPenetration = overlapY; axis = 'y'; }
            if (overlapZ < minPenetration) { minPenetration = overlapZ; axis = 'z'; }

            // Constantes físicas más conservadoras
            const float restitution = 0.05f; // Reducido para menos rebote
            const float positionCorrection = 1.2f; // Aumentado para separación completa
            const float velocityDamping = 0.5f; // Más agresivo
            const float angularDamping = 0.8f;

            Vector3 collisionNormal;
            float separationDistance = minPenetration * positionCorrection;

            // Separar objetos y calcular normal
            switch (axis) {
            case 'x':
                if (a->Position.x > b->Position.x) {
                    a->Position.x += separationDistance;
                    collisionNormal = Vector3(1.0f, 0.0f, 0.0f);
                }
                else {
                    a->Position.x -= separationDistance;
                    collisionNormal = Vector3(-1.0f, 0.0f, 0.0f);
                }
                break;

            case 'y':
                if (a->Position.y > b->Position.y) {
                    a->Position.y += separationDistance;
                    collisionNormal = Vector3(0.0f, 1.0f, 0.0f);
                }
                else {
                    a->Position.y -= separationDistance;
                    collisionNormal = Vector3(0.0f, -1.0f, 0.0f);
                }
                break;

            case 'z':
                if (a->Position.z > b->Position.z) {
                    a->Position.z += separationDistance;
                    collisionNormal = Vector3(0.0f, 0.0f, 1.0f);
                }
                else {
                    a->Position.z -= separationDistance;
                    collisionNormal = Vector3(0.0f, 0.0f, -1.0f);
                }
                break;
            }

            // Resolver velocidad lineal SOLO si se mueve hacia la superficie
            float velocityAlongNormal = MMaths::DotProduct(a->Velocity, collisionNormal);

            // CLAVE: Solo aplicar corrección si hay velocidad significativa hacia la superficie
            if (velocityAlongNormal < -0.1f) { // Threshold más alto
                // Aplicar restitución solo a la componente normal
                float newNormalVelocity = -velocityAlongNormal * restitution;
                Vector3 velocityChange = collisionNormal * (newNormalVelocity - velocityAlongNormal);
                a->Velocity = a->Velocity + velocityChange;
            }
            else if (velocityAlongNormal < 0.0f) {
                // Si la velocidad es muy pequeña, simplemente detenerla en esa dirección
                Vector3 normalVelocity = collisionNormal * velocityAlongNormal;
                a->Velocity = a->Velocity - normalVelocity;
            }

            // Aplicar amortiguación general DESPUÉS de la corrección
            a->Velocity = a->Velocity * velocityDamping;

            // TORQUE CORREGIDO - Solo en colisiones laterales, no en suelo
            if (axis != 'y' || a->Position.y <= b->Position.y) {
                Vector3 contactPoint = a->Position + collisionNormal * (-minPenetration * 0.5f);
                Vector3 r = contactPoint - a->Position;

                // Solo aplicar torque si hay velocidad tangencial significativa
                Vector3 contactVelocity = a->Velocity + MMaths::CrossProduct(a->AngularVelocity, r);
                Vector3 tangentialVel = contactVelocity - collisionNormal * MMaths::DotProduct(contactVelocity, collisionNormal);

                if (MMaths::Length(tangentialVel) > 1.0f) { // Threshold aún más alto
                    Vector3 torque = MMaths::CrossProduct(r, tangentialVel * -0.05f); // Más suave
                    a->AngularVelocity = a->AngularVelocity + torque;
                }
            }

            // Enderezamiento SOLO en superficies horizontales y SOLO si está sobre la superficie
            if (axis == 'y' && a->Position.y > b->Position.y) {
                Vector3 currentUp = MMaths::RotateVectorByQuaternion(Vector3(0, 1, 0), a->Rotation);
                float uprightness = MMaths::DotProduct(currentUp, Vector3(0, 1, 0));

                // Solo enderezar si está MUY inclinado y el objeto está relativamente quieto
                if (uprightness < 0.3f && MMaths::Length(a->Velocity) < 2.0f) {
                    Vector3 uprightTorque = MMaths::GetUprightTorque(a->Rotation);
                    a->AngularVelocity = a->AngularVelocity + uprightTorque * 0.01f; // Aún más suave
                }
            }

            // Amortiguación angular fuerte
            a->AngularVelocity = a->AngularVelocity * angularDamping;

            // Límite de velocidad angular más restrictivo
            float maxAngularSpeed = 2.0f; // Reducido
            float angularMagnitude = MMaths::Length(a->AngularVelocity);
            if (angularMagnitude > maxAngularSpeed) {
                a->AngularVelocity = a->AngularVelocity * (maxAngularSpeed / angularMagnitude);
            }

            // Sistema de detención mejorado con histéresis
            float linearSpeed = MMaths::Length(a->Velocity);

            if (linearSpeed < 0.2f && angularMagnitude < 0.3f) {
                // Aplicar amortiguación fuerte cuando está casi quieto
                a->Velocity = a->Velocity * 0.3f;
                a->AngularVelocity = a->AngularVelocity * 0.2f;

                // Parar completamente si está casi inmóvil
                if (linearSpeed < 0.05f && angularMagnitude < 0.08f) {
                    // En superficies horizontales, detener completamente la velocidad Y hacia abajo
                    if (axis == 'y' && a->Position.y > b->Position.y && a->Velocity.y < 0.0f) {
                        a->Velocity.y = 0.0f;
                    }
                    else if (linearSpeed < 0.02f) {
                        a->Velocity = Vector3(0, std::max(0.0f, a->Velocity.y), 0); // Solo mantener gravedad positiva
                    }
                    a->AngularVelocity = Vector3(0, 0, 0);
                }
            }

            // NUEVO: Prevenir re-entrada en el mismo frame
            // Marcar que este objeto ya fue procesado (necesitarías agregar un flag temporal)
            // a->ProcessedThisFrame = true;
        }

        if (b->BodyType == bDynamic && a->BodyType == bStatic) {
            ResolveCollision(b, a);
        }
    }

    static bool CheckAABB(const MBody* a, const MBody* b) {
        if (!a->AttachShaped || !b->AttachShaped)
            return false; 

        Vector3 aMin = a->Position - a->AttachShaped->Size;
        Vector3 aMax = a->Position + a->AttachShaped->Size;

        Vector3 bMin = b->Position - b->AttachShaped->Size;
        Vector3 bMax = b->Position + b->AttachShaped->Size;

        return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
            (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
            (aMin.z <= bMax.z && aMax.z >= bMin.z);
    }
};