#version 330 core

void main() {
    // Escribir explícitamente la profundidad para asegurar compatibilidad
    gl_FragDepth = gl_FragCoord.z;
}