#include "pch.h"
#include "Game.h"
#include "FileSystem.h"
#include "Graphics/Render.h"
#include "Resources.h"
#include "Editor/Editor.h"

namespace Inferno::Game {
    void LoadLevel(Inferno::Level&& level) {
        Inferno::Level backup = Level;

        try {
            assert(level.FileName != "");
            bool reload = level.FileName == Level.FileName;

            bool forceReload =
                level.IsDescent2() != Level.IsDescent2() ||
                Resources::CustomResources.Any() ||
                !String::InvariantEquals(level.Palette, Level.Palette);

            Rooms.clear();
            IsLoading = true;

            Level = std::move(level); // Move to global so resource loading works properly
            Resources::LoadLevel(Level);

            if (forceReload || Resources::CustomResources.Any()) // Check for custom textures before or after load
                Render::Materials->Unload();

            Render::Materials->LoadLevelTextures(Level, forceReload);
            Render::LoadLevel(Level);
            IsLoading = false;

            //Rooms = CreateRooms(Level);
            if (Settings::Editor.Descent3Mode)
                Editor::EnableDescent3Mode();

            Editor::OnLevelLoad(reload);
            Render::Materials->Prune();
            Render::Adapter->PrintMemoryUsage();
        }
        catch (const std::exception&) {
            Level = backup; // restore the old level if something went wrong
            throw;
        }
    }

    void LoadMission(const filesystem::path& file) {
        Mission = HogFile::Read(FileSystem::FindFile(file));
    }

    // Tries to read the mission file (msn / mn2) for the loaded mission
    Option<MissionInfo> TryReadMissionInfo() {
        try {
            if (!Mission) return {};
            auto path = Mission->GetMissionPath();
            MissionInfo mission{};
            if (!mission.Read(path)) return {};
            return mission;
        }
        catch (const std::exception& e) {
            SPDLOG_ERROR(e.what());
            return {};
        }
    }
}
