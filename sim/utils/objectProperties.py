import bpy  # type: ignore


def update_role_visual(self, context):
    obj = context.object
    if not obj:
        return

    if self.role in bpy.data.materials:
        mat = bpy.data.materials[self.role]
    else:
        mat = bpy.data.materials.new(name=self.role)
        if self.role == "TAG":
            mat.diffuse_color = (0, 1, 0, 1)  # green
        elif self.role == "ANCHOR":
            mat.diffuse_color = (0, 1, 1, 1)  # blue

    obj.data.materials.clear()

    if self.role == "TAG":
        obj.empty_display_type = 'SPHERE'
        obj.data.materials.append(mat)
        obj["Change_role_flag"] = True
    elif self.role == "ANCHOR":
        obj.empty_display_type = 'CUBE'
        obj.data.materials.append(mat)
        obj["Change_role_flag"] = True
    else:
        obj.empty_display_type = 'CONE'
        obj["Change_role_flag"] = True
    



class UWBDeviceProperties(bpy.types.PropertyGroup):
    role: bpy.props.EnumProperty(
        name="UWB Role",
        description="Tag or Anchor for this object",
        items=[
            ("TAG", "Tag", "Operate as Tag"),
            ("ANCHOR", "Anchor", "Operate as Anchor"),
            ("NONE", "None", "Don't participate in the process")
        ],
        default="NONE",
        update=update_role_visual
    )  # type: ignore


def register():
    bpy.utils.register_class(UWBDeviceProperties)
    bpy.types.Object.serial_props = bpy.props.PointerProperty(
        type=UWBDeviceProperties)


def unregister():
    del bpy.types.Object.serial_props
    bpy.utils.unregister_class(UWBDeviceProperties)
