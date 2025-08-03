function OnTick ()
    local pos = self():getPosition()
    pos.x = pos.x + 0.001

    self():setPosition(pos)

    local pb = self():getPhysicalObject()

    if pb ~= nil and self():getPosition().y <= -10 then
        pb:addForce(vector3.new(0, 1000, 0), ForceMode.Force)
        print("pb found")
    end
end
