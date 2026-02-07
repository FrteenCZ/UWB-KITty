import bpy
import blf
from bpy_extras.view3d_utils import location_3d_to_region_2d


def draw_distances(self, context):
    obj = context.active_object
    if obj:
        region = context.region
        rv3d = context.space_data.region_3d
        collection_name = "DistanceTargets"

        blf.size(0, 20)

        print("=== Distance Measurements ===")

        for target in bpy.data.collections.get(collection_name, []).objects:
            if target == obj:
                continue

            # Print the data to the console to emulate the serial output
            print(f"{target.name}: {(target.location - obj.location).length:.2f}m")

            # World-space position of both objects
            world_pos = target.location
            screen_pos = location_3d_to_region_2d(region, rv3d, world_pos)

            if screen_pos:
                # Draw the distance text on screen
                dist = (target.location - obj.location).length
                blf.position(0, screen_pos.x, screen_pos.y, 0)
                blf.draw(0, f"{target.name}: {dist:.2f}m")
