#pragma once
#include "CustomResourceLibrary.h"
#include "Level.h"
#include "Pig.h"
#include "HamFile.h"
#include "Mission.h"
#include "Hog2.h"
#include "OutrageBitmap.h"
#include "OutrageModel.h"
#include "OutrageTable.h"
#include "SoundSystem.h"

// Abstraction for game resources
namespace Inferno::Resources {
    void Init();

    inline CustomResourceLibrary CustomResources;

    extern SoundFile SoundsD1, SoundsD2;
    Sound::SoundResource GetSoundResource(SoundID id);
    string_view GetSoundName(SoundID id);

    const Palette& GetPalette();
    WClipID GetWallClipID(LevelTexID);
    const WallClip& GetDoorClip(WClipID);
    const WallClip* TryGetWallClip(WClipID);
    //const WallClip* TryGetWallClip(LevelTexID);

    const Powerup& GetPowerup(int id);
    const VClip& GetVideoClip(VClipID);
    const EffectClip& GetEffectClip(EClipID);
    const EffectClip& GetEffectClip(TexID);
    const EffectClip& GetEffectClip(LevelTexID);

    EClipID GetEffectClipID(LevelTexID);
    EClipID GetEffectClipID(TexID);

    const LevelTexture* TryGetLevelTextureInfo(LevelTexID);
    const LevelTexture& GetLevelTextureInfo(LevelTexID);
    const LevelTexture& GetLevelTextureInfo(TexID);
    LevelTexID GetDestroyedTexture(LevelTexID);
    const PigEntry& GetTextureInfo(TexID);
    const PigEntry* TryGetTextureInfo(TexID);
    const PigEntry* TryGetTextureInfo(LevelTexID);

    const PigEntry& GetTextureInfo(LevelTexID);
    const Model& GetModel(ModelID);
    const RobotInfo& GetRobotInfo(uint);

    List<TexID> CopyLevelTextureLookup();
    TexID LookupTexID(LevelTexID);
    TexID LookupModelTexID(const Model&, int16);

    string GetRobotName(uint id);
    // Can return none if the powerup is unused
    Option<string> GetPowerupName(uint id);

    // Loads the corresponding resources for a level
    void LoadLevel(Level&);

    // Returns bitmap data for a TexID
    const PigBitmap& GetBitmap(TexID);

    // Returns a modifiable bitmap
    PigBitmap& AccessBitmap(TexID);

    int GetTextureCount();

    inline HamFile GameData = {};

    // Reads a level from the mounted mission
    Level ReadLevel(string name);

    inline bool HasGameData() { return !GameData.Robots.empty() && !GameData.LevelTexIdx.empty(); }

    bool FoundDescent1();
    bool FoundDescent2();
    bool FoundVertigo();
    bool FoundDescent3();

    inline Hog2 Descent3Hog, Mercenary;
    inline Outrage::GameTable GameTable;
    inline List<Outrage::VClip> VClips; // Expanded from OAF headers

    void MountDescent3();

    Option<StreamReader> OpenFile(const string& name);

    Option<Outrage::Bitmap> ReadOutrageBitmap(const string& name);
    Option<Outrage::Bitmap> ReadOutrageVClip(const string& name);
    Option<Outrage::Model> ReadOutrageModel(const string& name);

    Outrage::Model const* GetOutrageModel(const string& name);

    // Loads D1 and D2 sounds
    void LoadSounds();

    struct PaletteInfo {
        string Name;
        string FileName;
    };

    span<PaletteInfo> GetAvailablePalettes();
}
