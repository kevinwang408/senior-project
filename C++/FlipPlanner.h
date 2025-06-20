// FlipPlanner.h
// ===========================================================================
// Defines structures and functions to plan flip shots (wall-bounce shots)
// in case direct child ball-to-hole paths are obstructed.
//
// Key logic:
// - Reflect target ball across wall to find mirror image
// - Connect cue ball to mirror and simulate bounce point
// - Validate path segments against obstacles
// ===========================================================================

#ifndef FLIP_PLANNER_H
#define FLIP_PLANNER_H

#include <vector>

// ---------------------------------------------------------------------------
// Structure representing a valid flip shot (wall-bounce assisted shot):
// - cue_to_wall_vector: vector from cueball to wall contact point
// - wall_contact_point: where the cueball should hit the wall
// - wall_to_target_vector: vector from wall to target ball
// - target_coords: location of child ball
// - hole_coords: intended hole (can be filled later)
// - total_distance: sum of cue->wall and wall->target lengths (for ranking)
// ---------------------------------------------------------------------------
struct FlipShot {
    std::vector<double> cue_to_wall_vector;
    std::vector<double> wall_contact_point;
    std::vector<double> wall_to_target_vector;
    std::vector<double> target_coords;
    std::vector<double> hole_coords;
    double total_distance;
};

// ---------------------------------------------------------------------------
// Evaluates all flip shots by mirroring each target across each wall,
// then computing potential cueball path to contact point and checking
// whether wall bounce path is feasible without collision.
//
// Parameters:
// - cueball_pos: position of the cueball (mother ball)
// - candidates: target child balls
// - obstacles: other balls (used to detect collision)
// - walls: fixed points representing potential bounce surfaces
// - bound_radius: clearance margin (typically ball diameter)
//
// Returns a list of valid FlipShot objects (can be ranked by distance)
// ---------------------------------------------------------------------------
std::vector<FlipShot> evaluateFlipShots(
    const std::vector<double>& cueball_pos,
    const std::vector<std::vector<double>>& candidates,
    const std::vector<std::vector<double>>& obstacles,
    const std::vector<std::vector<double>>& walls,
    double bound_radius
);

#endif // FLIP_PLANNER_H
