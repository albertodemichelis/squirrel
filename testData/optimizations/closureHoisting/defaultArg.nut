{
    let $ch0 = FUNCTION @ (this, v) {
      RETURN v
    }
    let x = FUNCTION @ (this, a = $ch0) {
      RETURN {
      }
    }
    let $ch1 = FUNCTION @ (this, v) {
      RETURN v
    }
    let c = CLASS {
      "constructor" <- FUNCTION constructor(this, a = $ch1) {
      }
    }
  }