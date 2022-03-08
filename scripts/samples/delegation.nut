local PEntity, PPlayer

PEntity = {
    name="noname"
    pos={x=0,y=0,z=0}
    type="entity"

    //methamethod
    function _typeof() {
        return type
    }

    function PrintPos() {
      println($"x={pos.x} y={pos.y} z={pos.z}")
    }

    function new(name, pos) {
      local newentity = clone PEntity
      if(name)
          newentity.name=name
      if(pos)
          newentity.pos=pos
      return newentity
  }
}


PPlayer = {
    model="warrior.mdl"
    weapon="fist"
    health=100
    armor=0
    //overrides the parent type
    type="player"

    function new(name,pos) {
        local p = clone PPlayer
        local newplayer = PEntity.new(name,pos)
        newplayer.setdelegate(p)
        return newplayer
    }
}


let player = PPlayer.new("godzilla",{x=10,y=20,z=30})

println($"PLAYER NAME: {player.name}")
println($"ENTITY TYPE: {typeof player}")

player.PrintPos()

player.pos.x=123

player.PrintPos()
