function Player:logic(dt)
    if Input.keyDown(70) then
        print("hello")
        self.setPos(vec3(0, 100, 0))
    end
end

function init()
    print("lua init")
end

function logic(dt)
    --print("logic")
end

function draw()
    --print("draw")
end
