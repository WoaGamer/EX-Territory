class CfgPatches
{
    class ExpansionTerritoryFuel
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data"};
    };
};

class CfgMods
{
    class ExpansionTerritoryFuel
    {
        dir = "ExpansionTerritoryFuel";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        name = "ExpansionTerritoryFuel";
        credits = "";
        author = "WoaGamer";
        version = "1.0";
        type = "mod";
        dependencies[] = {"Game","World","Mission"};
        class defs
        {
            class gameScriptModule
            {
                files[] = {"ExpansionTerritoryFuel/Scripts/3_Game"};
            };
            class worldScriptModule
            {
                files[] = {"ExpansionTerritoryFuel/Scripts/4_World"};
            };
            class missionScriptModule
            {
                files[] = {"ExpansionTerritoryFuel/Scripts/5_Mission"};
            };
        };
    };
};

class CfgVehicles
{
};
