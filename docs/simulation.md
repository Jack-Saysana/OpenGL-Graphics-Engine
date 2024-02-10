# Entity Simulation System
## Motivation
With an entity system defined for storing state data of model instances, an additional system must be defined in order to efficently detect and respond to collisions. Furthermore, in order to ensure collisions are calculated efficently, and without redundant computation, the ability to create multiple simulations, each with its own purpose, is also needed.

These simulations are represented by the `SIMULATION` struct. These seperate simulations track their own `COLLIDERS`, with helper functions created to facilitate integration, collision detection, and collision response. These simulations can also be used for different purposes, with one for physics simulation, another for combat simulation, and another for event triggers, for example.

## Global Variables

The simulation system makes usage of the `DELTA_TIME` global variable for integrating colliders. The onus is on the developer to continually update `DELTA_TIME` each frame so that integration may be supported.

## Initialization and Clean-Up

### Functions

```SIMULATION *init_sim(float max_extent, unsigned int max_depth)```

Initializes a new simulation for usage.

**Arguments**

- `float max_extent`: Half-width of the cuboid oct tree which contains all objects in the simulation

- `unsigned int max_depth`: Maximum depth of the oct tree which contains all objects in the simulation

**Returns**

A pointer to the new `SIMULATION` struct

```void free_sim(SIMULATION *sim)```

Deallocates a `SIMULATION` struct

**Arguments**

- `SIMULATION *sim`: Simulation to be deallocated

## Collider input/deletion

Once a simulation is initialized, the first step is to add colliders to the simulation. Colliders of an entity can be added to the simulation selectively, creating the ability to create multiple, purpose based simulations.

### Functions

```int sim_add_entity(SIMULATION *sim, ENTITY *entity, int collider_filter)```

Adds an entities colliders to a simulation, given some filter settings for the collider

**Arguements**

- `SIMULATION *sim`: Simulation to add the colliders to

- `ENTITY *entity`: Entity to add colliders from

- `int collider_filter`: Bitfield specifying which types of colliders from `entity` should be added to `sim`
  - The following options can be specified and or'ed (|) together
    - `ALLOW_DEFAULT`: All colliders of `entity` with type `DEFUALT` will be added
    - `ALLOW_HURT_BOXES`: All colliders of `entity` with type `HURT_BOX` will be added
    - `ALLOW_HIT_BOXES`: All colliders of `entity` with type `HIT_BOX` will be added

**Returns**

0 if successful, non-zero if an error occured

```int sim_remove_entity(SIMULATION *sim, ENTITY *entity)```

Removes all colliders in `sim` which belong to `entity`

**Arguements**

- `SIMULATION *sim`: Simulation to remove colliders from

- `ENTITY *entity`: Entity whose colliders are to be removed

**Returns**

0 if successful, non-zero if arguments are invalid

## Collision Detection

With entities present in a simulation, it is now possible to simulate collision. A simple interface is provided for detecting all collisions in a simulation at once. See the section below for some provided [collision resolution strategies](#physics)

### Structs

```COLLISION:```

A `COLLISION` struct is defined to act as an easy to use object for reading and responding to collisions from the simulation.

#### Members:

- `ENTITY *a_ent`: The first entity in the collision, guaranteed to have been moving at the time of the collision

- `ENTITY *b_ent`: The second entity in the collision, not guaranteed to have been moving at collision time

- `size_t a_offset`: Index of collider in `a_ent` which is involved in the collision. **Note**: Accessing this collider will only give the entity-space coordinates of the collider

- `size_t b_offset`: Index of the collider in `b_ent` which is involved in the collision. **Note**: Accessing this collider will only give the entity-space coordinates of the collider

- `COLLIDER a_world_col`: A copy of the first collider involved in the collision. However, this colliders vertices and center of mass is given in world coordinates.

- `COLLIDER b_world_col`: A copy of the second collider involved in the collision. However, this colliders vertices and center of mass is given in world coordinates.

- `vec3 col_dir`: The direction of collision from a to b, in world space

- `vec3 col_point`: The point of collision in world space

### Functions

```size_t get_sim_collisions(SIMULATION *sim, COLLISION **dest)```

For each collision detected in a simulation, a `COLLISION` is created and stored in a buffer allocated at `dest`.

**Arguements**

- `SIMULATION *sim`: Simulation to detect collisions in

- `COLLISION **dest`: Pointer to where to allocate destination buffer for storing each collision pair.

**Returns**

The number of collisions stored in the buffer created at `dest`

## Collider Integration / Updating

With colliders present in a simulation, each with their own physics data such as velocity and acceleration, an easy integration interface is provided to automatically move colliders throughout the simulation.

### Functions

```void prep_sim_movement(SIMULATION *sim)```

A prepratory function which should be called before any colliders in a simulation are moved. This and the `update_sim_movement()` function are used to ensure the oct-tree of each simulation is properly synced with the positions of the simulated colliders.

**Arguments**

- `SIMUATION *sim`: Simulation to prepare

```void update_sim_movement(SIMULATION *sim)```

A finalizing function to update the state of a simulation's oct-tree with the position info of all the simulated colliders. Should be called after a call to `prep_sim_movement()` and subsequent collider movement functionality.

**Arguments**

- `SIMULATION *sim`: Simulation to update

```void integrate_sim(SIMULATION *sim)```

For each `COLLIDER` in `sim`, its velocity and acceleration are integrated to ultimately update the colliders position in the simulation.

**Arguements**

- `SIMULATION *sim`: Simulation to integrate

```void refresh_collider(SIMULATION *sim, ENTITY *entity, size_t collider_offset)```

For the given collider, it's status in the simulation is refreshed. This call is necesarry in the case where an object exists within multiple different simulations, a and b. In the event simulation a causes the object to begin moving, `refresh_collider()` must be called on the collider with simulation b to ensure simulation b is synced with the current status of the collider.

**Arguments**

- `SIMULATION *sim`: Simulation in which to refresh the collider

- `ENTITY *entity`: Entity to whom the collider belongs to

- `size_t collider_offset`: Index of the collider within the entity

## Physics

Although optional, additional simulation helpers are provided in the way of simple physics solvers.

### Functions

```void impulse_resolution(SIMULATION *sim, COLLISION col)```

An optional collision resolution function which resolves a single collision in accordance to impulse-based collision resolution. The calculations used by this function is specified [here](https://en.wikipedia.org/wiki/Collision_response#Impulse-based_reaction_model).

**Arguements**

- `SIMULATION *sim`: The simulation in which to resolve `col`

- `COLLISION col`: A collision to resolve between two colliders

**Returns**

```void sim_add_force(SIMULATION *sim, vec3 force)```

By default, `SIMULATION`s do not enforce gravity or any sort of universal force, so when creating a physics simulation in a default `SIMULATION`, colliders float through space. This function allows for the specification of an acceleration vector, which will then impact every object in the simulation. **Note** This function is additive, meaning repeated calls simply add onto the current acceleration enforced by `sim`. See `sim_clear_force()` for clearing the already existing force of a `sim`.

**Arguements**

- `SIMULATION *sim`: The simulation to add the force to

- `vec3 force`: The "force" vector to add to the simulation. This vector acts as a uniform acceleration across all colliders in the simulation. For example, the vector `{ 0.0, -9.81, 0.0 }`, would approximate gravity in the simulation.

```void sim_clear_forces(SIMULATION *sim)```

Clears the force vector in a simulation to zero.

**Arguements**

- `SIMULATION *sim`: The simulation whose force should be cleared
