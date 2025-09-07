import bpy  # type: ignore
import numpy as np
import math
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

    # Update geometry
    mesh = obj.data
    mesh.clear_geometry()
    mesh.from_pydata([start, end], [(0, 1)], [])
    mesh.update()
    return obj


def update_scene(thread):
    # Trilateration State
    if thread.trilateration_cords is not None and thread.alpha is not None and thread.null_space is not None:
        [x, y, z] = thread.trilateration_cords[0]
        alpha = thread.alpha
        null_space = np.array(thread.null_space)

        coll = bpy.data.collections.get("Trilateration")
        if not coll:
            coll = bpy.data.collections.new("Trilateration")
            bpy.context.scene.collection.children.link(coll)

        # 3D subspace
        if len(null_space) == 0:
            cleanup_collection(coll, ["Solution"])
            obj = bpy.data.objects.get("Solution")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "Solution"
                obj.scale = (1.2, 1.2, 1.2)
                coll.objects.link(obj)

            obj.location = (x, y, z)

        # 2D subspace
        elif len(null_space) == 1:
            cleanup_collection(
                coll, ["Center", "Normal", "Solution", "AntiNormal", "AntiSolution"])
            obj = bpy.data.objects.get("Center")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "Center"
                obj.scale = (0.1, 0.1, 0.1)
                coll.objects.link(obj)
            obj.location = (x, y, z)

            ensure_arrow("Normal", (x, y, z),
                         null_space[0] * alpha, relative=True, coll=coll)
            obj = bpy.data.objects.get("Solution")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "Solution"
                obj.scale = (1.2, 1.2, 1.2)
                coll.objects.link(obj)
            obj.location = (x, y, z) + alpha * null_space[0]

            ensure_arrow("AntiNormal", (x, y, z), -
                         null_space[0] * alpha, relative=True, coll=coll)
            obj = bpy.data.objects.get("AntiSolution")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "AntiSolution"
                obj.scale = (1.2, 1.2, 1.2)
                coll.objects.link(obj)
            obj.location = (x, y, z) - alpha * null_space[0]

        # 1D subspace
        elif len(null_space) == 2:
            cleanup_collection(coll, ["Center", "1DSolutionSpace"])
            obj = bpy.data.objects.get("Center")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "Center"
                obj.scale = (0.1, 0.1, 0.1)
                coll.objects.link(obj)
            obj.location = (x, y, z)

            obj = bpy.data.objects.get("1DSolutionSpace")
            if obj is None:
                bpy.ops.object.empty_add(type='CIRCLE')
                obj = bpy.context.active_object
                obj.name = "1DSolutionSpace"
                coll.objects.link(obj)
            obj.location = (x, y, z)
            obj.scale = (alpha, alpha, alpha)
            normalVec = np.cross(null_space[0], null_space[1])
            r = np.sqrt(normalVec[0]**2 + normalVec[1]**2 + normalVec[2]**2)
            theta = np.arccos(normalVec[2] / r) if r != 0 else 0.0
            phi = np.arctan2(normalVec[1], normalVec[0])
            obj.rotation_euler = (np.pi/2 - theta, 0, phi - np.pi/2)


        # 0D subspace (BROKEN ON ESP!)
        elif len(null_space) == 3:
            cleanup_collection(coll, ["Center", "0DSolutionSpace"])
            obj = bpy.data.objects.get("Center")
            if obj is None:
                bpy.ops.object.empty_add(type='CUBE')
                obj = bpy.context.active_object
                obj.name = "Center"
                obj.scale = (0.1, 0.1, 0.1)
                coll.objects.link(obj)
            obj.location = (x, y, z)

            obj = bpy.data.objects.get("0DSolutionSpace")
            if obj is None:
                bpy.ops.mesh.primitive_uv_sphere_add()
                obj = bpy.context.active_object
                obj.name = "0DSolutionSpace"
                coll.objects.link(obj)
            obj.location = (x, y, z)
            obj.scale = (alpha, alpha, alpha)
            
    # Kalman Filter State
    if thread.kalman_cords is not None:
        [x, y, z, vx, vy, vz] = thread.kalman_cords[0]

        obj = bpy.data.objects.get("Kalman")
        if obj is None:
            bpy.ops.object.empty_add(type='PLAIN_AXES')
            obj = bpy.context.active_object
            obj.name = "Kalman"

        obj.location = (x, y, z)

        ensure_arrow("KalmanVelocity", (x, y, z),
                     (vx, vy, vz), relative=True)


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
        self._thread = SerialThread(port='/dev/ttyUSB0', baudrate=115200)
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
