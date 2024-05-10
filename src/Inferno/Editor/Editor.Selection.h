#pragma once
#include "Settings.h"
#include "Types.h"
#include "Level.h"
#include "Events.h"
#include "Camera.h"
#include "Command.h"

namespace Inferno::Editor {
    enum class SelectionMode {
        Segment,
        Face,
        Edge,
        Point,
        Object,
        Transform
    };

    // Transform orientations
    enum class CoordinateSystem {
        Local, // Depends on the selection. Normal for faces, orientation for objects.
        Global, // Axis aligned csys at the origin. World Space
        User // User defined and positioned
    };

    struct SelectionHit {
        Tag Tag;
        short Edge = 0;
        Vector3 Normal;
        float Distance = 0;
        ObjID Object = ObjID::None;
        bool operator<=>(const SelectionHit& rhs) const = default;

        bool operator==(const SelectionHit& rhs) const {
            return Tag == rhs.Tag && Edge == rhs.Edge && Object == rhs.Object;
        }
    };

    class EditorSelection {
        SelectionHit _selection = {};
        int _cycleDepth = 0;

    public:
        SegID Segment = SegID::None;
        SideID Side;
        uint16 Point; // 0 - 3
        List<SelectionHit> Hits;
        ObjID Object = ObjID::None;

        List<Tag> SegmentTags() const {
            return {
                { Segment, SideID::Left },
                { Segment, SideID::Top },
                { Segment, SideID::Right },
                { Segment, SideID::Bottom },
                { Segment, SideID::Back },
                { Segment, SideID::Front },
            };
        }

        Tag Tag() const { return { Segment, Side }; }
        PointTag PointTag() const { return { Segment, Side, Point }; }

        void Click(Level&, Ray ray, SelectionMode, bool includeInvisible);

        void Forward();
        void Back();

        void SelectLinked();

        void NextSide() {
            Side++;
            SetSelection({ Segment, Side });
        }

        void PreviousSide() {
            Side--;
            SetSelection({ Segment, Side });
        }

        void NextPoint() {
            if (Point == 3) Point = 0;
            else Point++;
            SetSelection({ Segment, Side });
        }

        void PreviousPoint() {
            if (Point == 0) Point = 3;
            else Point--;
            SetSelection({ Segment, Side });
        }

        void NextItem();
        void PreviousItem();

        // returns the transform origin of the selection
        Vector3 GetOrigin(SelectionMode) const;

        List<PointID> GetVertexHandles(Level&) const;
        Tuple<LevelTexID, LevelTexID> GetTextures() const;

        void SelectByTexture(LevelTexID);

        void Reset() {
            Segment = SegID::None;
            Object = ObjID::None;
        }

        void SetSelection(Inferno::Tag);
        void SetSelection(SegID id) { SetSelection({ id, Side }); }

        void SetSelection(ObjID id) {
            if (id < ObjID::None) id = ObjID::None;
            Object = id;
            Events::SelectObject();
        }

        void SelectByTrigger(TriggerID);
        void SelectByWall(WallID);
    };

    class MultiSelection {
    public:
        void Update(Level&, const Ray&);
        void UpdateFromWindow(Level&, Vector2 p0, Vector2 p1, const Camera& camera);

        List<PointID> GetVertexHandles(Level&);

        List<SegID> GetSegments(const Level&) const;

        Set<Tag> Faces;
        Set<SegID> Segments;
        Set<PointID> Points;
        Set<ObjID> Objects;

        bool operator==(const MultiSelection& rhs) const {
            return
                Faces == rhs.Faces &&
                Segments == rhs.Segments &&
                Points == rhs.Points &&
                Objects == rhs.Objects;
        }

        bool HasSelection(SelectionMode mode) const {
            switch (mode) {
                case SelectionMode::Segment:
                    return !Segments.empty();
                case SelectionMode::Edge:
                case SelectionMode::Point:
                    return !Points.empty();
                case SelectionMode::Object:
                    return !Objects.empty();
                default:
                    return !Faces.empty();
            }
        }

        // Gets the marked faces or converts segments into selected faces
        List<Tag> GetMarkedFaces(SelectionMode mode) const {
            List<Tag> faces;

            switch (mode) {
                case SelectionMode::Segment:
                    for (auto& id : Segments) {
                        for (auto& side : SideIDs)
                            faces.push_back({ id, side });
                    }
                    break;
                case SelectionMode::Edge:
                case SelectionMode::Point:
                case SelectionMode::Object:
                    break;

                default:
                    Seq::append(faces, Faces);
                    break;
            }

            return faces;
        }

        List<Tag> GetMarkedFaces() const { return GetMarkedFaces(Settings::Editor.SelectionMode); }

        // Returns the geometric center of the active marks
        Vector3 GetMarkedCenter(SelectionMode mode, Level& level) const {
            Vector3 center;
            int count = 0;

            switch (mode) {
                case SelectionMode::Segment:
                    for (auto& id : Segments) {
                        if (auto seg = level.TryGetSegment(id)) {
                            center += seg->Center;
                            count++;
                        }
                    }
                    break;
                case SelectionMode::Edge:
                case SelectionMode::Point:
                    for (auto& id : Points) {
                        if (auto v = level.TryGetVertex(id)) {
                            center += *v;
                            count++;
                        }
                    }
                    break;
                case SelectionMode::Face:
                    for (auto& face : Faces) {
                        if (auto f = level.TryGetSide(face)) {
                            center += f->Center;
                            count++;
                        }
                    }
                    break;

                case SelectionMode::Object:
                    for (auto& id : Objects) {
                        if (auto o = level.TryGetObject(id)) {
                            center += o->Position;
                            count++;
                        }
                    }
                    break;
            }

            if (count > 0)
                center /= (float)count;

            return center;
        }

        // Adjusts remaining selection after removing a segment
        void RemoveSegment(SegID id) {
            auto faces = Seq::ofSet(Faces);
            Faces.clear();

            for (auto& face : faces) {
                if (face.Segment == id) continue; // don't add faces from the deleted segment

                auto seg = face.Segment;
                if (seg >= id) seg--; // shift ids that are after the removed segment
                Faces.insert({ seg, face.Side });
            }

            Points.clear(); // this is too hard to deal with
        }

        void MarkAll();
        void InvertMarked();

        void Clear() {
            Faces.clear();
            Segments.clear();
            Points.clear();
            Objects.clear();
            Events::MarkedFacesChanged();
        }

        void ClearCurrentMode() {
            switch (Settings::Editor.SelectionMode) {
                case SelectionMode::Face: Faces.clear();
                    break;
                case SelectionMode::Segment: Segments.clear();
                    break;
                case SelectionMode::Point: Points.clear();
                    break;
                case SelectionMode::Object: Objects.clear();
                    break;
            }

            Events::MarkedFacesChanged();
        }

        void ToggleMark();
    };

    inline MultiSelection Marked;
    inline EditorSelection Selection;

    Option<Tuple<int16, int16>> FindSharedEdges(Level&, Tag src, Tag dest);
    bool HasVisibleTexture(Level&, Tag);

    inline List<Tag> FacesForSegment(SegID seg) {
        List<Tag> faces;
        for (auto& side : SideIDs) {
            faces.push_back({ seg, side });
        }

        return faces;
    }

    // Returns all faces of the segments
    inline List<Tag> FacesForSegments(span<SegID> segs) {
        List<Tag> faces;
        for (auto& seg : segs) {
            for (auto& side : SideIDs) {
                faces.push_back({ seg, side });
            }
        }

        return faces;
    }

    // Executes a function on each valid marked object
    void ForMarkedObjects(const std::function<void(Object&)>& fn);

    namespace Commands {
        void MarkCoplanar(Tag);
        void SelectTexture(bool usePrimary, bool useSecondary);

        extern Command ToggleMarked, ClearMarked, MarkAll, InvertMarked;
    }
}
