import bpy
import blf


def draw_coordinates(self, context):
    obj = context.active_object
    if obj:
        x = obj.location.x
        y = obj.location.y
        z = obj.location.z
        blf.size(0, 20)
        blf.position(0, 50, 50, 0)
        blf.draw(0, f"X Position: {x:.2f}")

        blf.position(0, 50, 80, 0)
        blf.draw(0, f"Y Position: {y:.2f}")

        blf.position(0, 50, 110, 0)
        blf.draw(0, f"Z Position: {z:.2f}")

    else:
        blf.position(0, 50, 50, 0)
        blf.size(0, 20)
        blf.draw(0, f"Object '{obj.name}' not found")
