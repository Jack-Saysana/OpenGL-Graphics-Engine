# Model loading and instancing
## Motivation
The entire purpose of the engine is based around the manipulation of objects in 3D space. It is very convenient to import premade 3D models with mesh, animation and physics data to represent these objects when rendered to the screen. With this, inevitably comes the instance where a program utilizes multiple instances of a singular 3D model, say a simple character mesh, for purposes such as a physics simulation. These characters will have identical 3D models, animations and hitboxes, but should have the ability to have differing spatial positions, animation state, and physical properties. As such, a system must be devised to separate information that is uniform across all simulated objects represented by the same model, and object specific information.

These individual objects are referred to as **entities** by the engine, and they can be thought of as instances of **models**, which contain the entity's essential, unchanging information.

Each **entity** in the world has their own:
- Spatial attributes (position, rotation, scale)
- Skeletal bone orientations
- Physical properties

While each **model** contains:
- Mesh data
- Available animations
- Hitboxes
- Bone structure

With this structure, objects that are all instances of the same 3D model do not have to needlessly hold an instance of repetitive mesh information, and instead, only need to maintain information that is exclusive to the state of that object.

## Loading

A model can be loaded via the `load_model` function. These models are read from modified .obj files. These files are similar in format to Wavefront .obj files. However, they are expanded to contain bone, animation and collider data. These files may be exported from blender utilizing a [custom python script](https://github.com/Jack-Saysana/Blender-Custom-Obj-Exporter). These files are preprocessed by the engine to create a model binary, which contains the same information as these .obj files, but may be read at much faster speeds in the future. However, the binaries are not guarenteed to be portable across systems.

### Functions

```MODEL *load_model(char *path)```

Loads a model from a modified .obj file. If no preprocessed binary of the same name with a ".bin" extention exists in the directory in which the .obj file resides, the engine will also preprocess the file and create a preprocessed binary for quick reading.

**Arguments**

- `char *path`: path to the model's .obj file

**Returns**

A pointer to the imported model or NULL if an error has occured.

## Instancing

Once imported, models can be instanced to create "entities" with their own animation, physics and spatial state. These entities can then be entered into physical simulations for more complex scenes.

### Functions

```ENTITY *init_entity(MODEL *model)```

Creates an instance of a model with its own animation, physics and spatial state.

**Arguments**

- `MODEL *model`: Pointer to the model to be instanced

**Returns**

A pointer to the new entity instance or NULL if an error has occured

## Control

TODO

## Drawing

Both the ENTITY and MODEL type can be rendered to the screen via their respective draw functions. Furthermore, debugging draw functions are also provided to render entity data such as colliders and bones.

### Functions

```void draw_model(unsigned int shader, MODEL *model)```

Draws a model to the screen. By default, the mesh will be rendered as it appears straight from the binary file, so the matricies used to render the model must be manually set.

**Arguments**

- `unsigned int shader`: The ID of the shader program used to render the model
  - The model has the following VBO structure which should be reflected in the shader:
    - layoutfloat vertex[3]: coordinate vertices
    - float normal[3]: vertex normal
    - float tex_coords[2]: texture coordinates
    - int bone_ids[4]: indicies of 4 (or less) bones which are bound to the vertex
      - Will be -1 if bone slot not used
    - float weights[4]: Corresponding weights of influence from each bone in `bone_ids`

- `MODEL *model`: Pointer to the model to be render

```void draw_entity(unsigned int shader, ENTITY *entity)```

Draws an entity to the screen, using the model that was used to initialize the entity. The physics, general position, bone orientation, and transformation data is automatically applied to the model upon rendering the entity, so no model matricies must be manually set.

**Arguments**

- `unsigned int shader`: The ID of the shader program used to render the model
  - The model has the following VBO structure which should be reflected in the shader:
    - layoutfloat vertex[3]: coordinate vertices
    - float normal[3]: vertex normal
    - float tex_coords[2]: texture coordinates
    - int bone_ids[4]: indicies of 4 (or less) bones which are bound to the vertex
      - Will be -1 if bone slot not used
    - float weights[4]: Corresponding weights of influence from each bone in `bone_ids`

- `ENTITY *entity`: Pointer to the entity to render

## Cleanup

Upon concluding their usage, both entities and models must be deallocated with their respective functions.

### Functions

```void free_model(MODEL *model)```

Deallocates a model

**Arguments**

`MODEL *model`: Pointer to model to be deallocated

```void free_entity(ENTITY *entity)```

Deallocates an entity. Does **NOT** deallocate the model which the entity is an instance of.
