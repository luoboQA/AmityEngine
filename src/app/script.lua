-- ==========================================
-- AmityEngine Lua Scripting Test
-- ==========================================

print("[LUA] Amity Script Loaded Successfully!")
print("[LUA] Press [SPACE] to sail forward, or [R] to reset the ship's position.")

-- Get a reference to our pirate ship entity object!
local pirateShip = GetEntity("pirateShip")

ConnectKeyBegan(function(keycode)
    print("[LUA] KeyPressed Event fired! Keycode: " .. keycode)

    -- Space Key (KeyCode 32): Sail the ship forward!
    if keycode == 32 then
        print("[LUA] Space pressed! Sailing pirateShip forward.")
        -- Moves the ship forward along -Z direction by setting velocity
        pirateShip:SetVelocity(0.0, 0.0, -100.0)
    end

    -- R Key (KeyCode 82): Reset position & velocity!
    if keycode == 82 then
        print("[LUA] R pressed! Resetting pirateShip position and velocity.")
        pirateShip:SetPosition(0.0, -9.0, 0.0)
        pirateShip:SetVelocity(0.0, 0.0, 0.0)
    end
end)
