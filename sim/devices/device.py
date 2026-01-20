

class Device:
    def __init__(self, blender_object=None, role="NONE"):
        self.obj = blender_object
        self.role = role
        
        self.null_space = []
        self.alpha = 0.0
        self.trilateration = []
        self.kalman = []
        self.last_update = 0.0

        self.pos = [0.0, 0.0, 0.0] # position from trilateration
        self.vel = [0.0, 0.0, 0.0] # velocity from kalman
        self.epos = [0.0, 0.0, 0.0] # extrapolated position from kalman