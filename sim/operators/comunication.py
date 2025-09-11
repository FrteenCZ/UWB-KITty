import bpy  # type: ignore
import numpy as np
import os
from ..utils.ESPcom import SerialThread


def cleanup_collection(coll, keep_names=None):
    keep_names = set(keep_names or [])

    for obj in list(coll.objects):
        if obj.name not in keep_names:
            bpy.data.objects.remove(obj, do_unlink=True)


def ensure_arrow(name, start=(0, 0, 0), end=(0, 0, 0), relative=False, coll=None):
    """Create or update an arrow (mesh line) between start and end."""
    if relative:
        end = (start[0] + end[0], start[1] + end[1], start[2] + end[2])

    obj = bpy.data.objects.get(name)
    if obj is None:
        # Create a mesh line
        mesh = bpy.data.meshes.new(name + "Mesh")
        obj = bpy.data.objects.new(name, mesh)
        bpy.context.collection.objects.link(obj)
        if coll is not None:
            coll.objects.link(obj)
            collRM = bpy.data.collections.get("Collection")
            if collRM and obj.name in collRM.objects:
                collRM.objects.unlink(obj)

    # Update geometry
    mesh = obj.data
    mesh.clear_geometry()
    mesh.from_pydata([start, end], [(0, 1)], [])
    mesh.update()
    return obj


def ensure_empty(name, type, scale=None, location=None, rotation=None, coll=None):
    obj = bpy.data.objects.get(name)
    if obj is None:
        bpy.ops.object.empty_add(type=type)
        obj = bpy.context.active_object
        obj.name = name
        if coll is not None:
            coll.objects.link(obj)
            collRM = bpy.data.collections.get("Collection")
            if collRM and obj.name in collRM.objects:
                collRM.objects.unlink(obj)

    if scale is not None:
        obj.scale = scale

    if location is not None:
        obj.location = location

    if rotation is not None:
        obj.rotation_euler = rotation


def update_scene(thread):
    # Trilateration State
    if thread.trilateration_cords is not None and thread.alpha is not None and thread.null_space is not None:
        [x, y, z] = thread.trilateration_cords[0]
        alpha = thread.alpha
        null_space = np.array(thread.null_space)

        collTrilat = bpy.data.collections.get("Trilateration")
        if not collTrilat:
            collTrilat = bpy.data.collections.new("Trilateration")
            bpy.context.scene.collection.children.link(collTrilat)

        # 3D subspace
        if len(null_space) == 0:
            cleanup_collection(
                collTrilat, ["Solution"])

            ensure_empty("Solution", "CUBE", scale=(1.2, 1.2, 1.2),
                         location=(x, y, z), coll=collTrilat)

        # 2D subspace
        elif len(null_space) == 1:
            cleanup_collection(
                collTrilat, ["Center", "Normal", "Solution", "AntiNormal", "AntiSolution"])

            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=collTrilat)

            ensure_arrow("Normal", (x, y, z),
                         null_space[0] * alpha, relative=True, coll=collTrilat)
            ensure_empty("Solution", "CUBE", scale=(1.2, 1.2, 1.2), location=(
                x, y, z) + alpha * null_space[0], coll=collTrilat)

            ensure_arrow("AntiNormal", (x, y, z), -
                         null_space[0] * alpha, relative=True, coll=collTrilat)
            ensure_empty("AntiSolution", "CUBE", scale=(1.2, 1.2, 1.2), location=(
                x, y, z) - alpha * null_space[0], coll=collTrilat)

        # 1D subspace
        elif len(null_space) == 2:
            cleanup_collection(
                collTrilat, ["Center", "1DSolutionSpace"])

            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=collTrilat)

            normalVec = np.cross(null_space[0], null_space[1])
            r = np.sqrt(normalVec[0]**2 + normalVec[1]**2 + normalVec[2]**2)
            theta = np.arccos(normalVec[2] / r) if r != 0 else 0.0
            phi = np.arctan2(normalVec[1], normalVec[0])
            ensure_empty("1DSolutionSpace", "CIRCLE", scale=(alpha, alpha, alpha), location=(
                x, y, z), rotation=(np.pi/2 - theta, 0, phi - np.pi/2), coll=collTrilat)

        # 0D subspace (BROKEN ON ESP!)
        elif len(null_space) == 3:
            cleanup_collection(
                collTrilat, ["Center", "0DSolutionSpace"])

            ensure_empty("Center", "CUBE", scale=(0.1, 0.1, 0.1),
                         location=(x, y, z), coll=collTrilat)

            obj = bpy.data.objects.get("0DSolutionSpace")
            if obj is None:
                bpy.ops.mesh.primitive_uv_sphere_add()
                obj = bpy.context.active_object
                obj.name = "0DSolutionSpace"
                collTrilat.objects.link(obj)
                collRM = bpy.data.collections.get("Collection")
                if collRM and obj.name in collRM.objects:
                    collRM.objects.unlink(obj)
            obj.location = (x, y, z)
            obj.scale = (alpha, alpha, alpha)

    # Kalman Filter State
    if thread.kalman_cords is not None:
        [x, y, z, vx, vy, vz] = thread.kalman_cords[0]

        collKalman = bpy.data.collections.get("Kalman")
        if not collKalman:
            collKalman = bpy.data.collections.new("Kalman")
            bpy.context.scene.collection.children.link(collKalman)

        ensure_empty("Kalman", "PLAIN_AXES",
                     location=(x, y, z), coll=collKalman)

        ensure_arrow("KalmanVelocity", (x, y, z),
                     (vx, vy, vz), relative=True, coll=collKalman)


def collect_points():
    points = []
    obj = bpy.data.objects.get("Cube")  # your tag
    if not obj:
        return points

    collection = bpy.data.collections.get("DistanceTargets")
    if not collection:
        return points

    for target in collection.objects:
        if target == obj:
            continue

        points.append({
            "name": target.name,
            "location": {
                "x": target.location.x,
                "y": target.location.y,
                "z": target.location.z,
            },
            "distance": (target.location - obj.location).length
        })
    return points


class SERIAL_OT_ModalESP(bpy.types.Operator):
    """Continuously read from ESP"""
    bl_idname = "wm.serial_modal_esp"
    bl_label = "ESP Serial Modal Operator"

    _timer = None
    _thread = None

    def modal(self, context, event):
        if event.type == 'TIMER':
            points = collect_points()
            self._thread.send_trilateration(points)

            update_scene(self._thread)

        if not self._thread.keep_running:
            self.cancel(context)
            return {'CANCELLED'}

        return {'PASS_THROUGH'}

    def execute(self, context):
        devices = [
            f"/dev/{d}" for d in os.listdir("/dev") if d.startswith("ttyUSB")]

        if not devices:
            self.report({'ERROR'}, "No serial devices found (no /dev/ttyUSB*)")
            return {'CANCELLED'}

        self._thread = SerialThread(port=devices[0], baudrate=115200)
        self._thread.start()

        wm = context.window_manager
        self._timer = wm.event_timer_add(0.5, window=context.window)
        wm.modal_handler_add(self)
        return {'RUNNING_MODAL'}

    def cancel(self, context):
        if self._thread:
            self._thread.stop()
            self._thread.join()
        wm = context.window_manager
        wm.event_timer_remove(self._timer)
        print("Modal operator stopped.")


def menu_func(self, context):
    self.layout.operator(SERIAL_OT_ModalESP.bl_idname)


def register():
    bpy.utils.register_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.append(menu_func)


def unregister():
    bpy.utils.unregister_class(SERIAL_OT_ModalESP)
    bpy.types.TOPBAR_MT_file.remove(menu_func)
