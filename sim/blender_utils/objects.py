import bpy  # type: ignore


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
