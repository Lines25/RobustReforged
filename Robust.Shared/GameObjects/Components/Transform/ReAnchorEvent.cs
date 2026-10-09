using System.Numerics;

namespace Robust.Shared.GameObjects;

/// <summary>
/// Raised when an entity is re-anchored to another grid.
/// </summary>
[ByRefEvent]
public readonly struct ReAnchorEvent
{
    public readonly EntityUid Entity;
    public readonly EntityUid OldGrid;
    public readonly EntityUid Grid;
    public readonly TransformComponent Xform;

    /// <summary>
    /// Tile on both the old and new grid being re-anchored.
    /// </summary>
    public readonly Vector2i TilePos;

    public ReAnchorEvent(EntityUid uid, EntityUid oldGrid, EntityUid grid, Vector2i tilePos, TransformComponent xform)
    {
        Entity = uid;
        OldGrid = oldGrid;
        Grid = grid;
        TilePos = tilePos;
        Xform = xform;
    }
}
