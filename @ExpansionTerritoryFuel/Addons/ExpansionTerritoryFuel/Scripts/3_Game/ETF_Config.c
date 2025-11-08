class ETF_Config
{
    static ref ETF_Config m_Instance;

    ref ETFDisplayCfg Display;
    ref ETFFormattingCfg Formatting;

    static ETF_Config Get()
    {
        if (!m_Instance)
            m_Instance = new ETF_Config();
        return m_Instance;
    }

    void ETF_Config()
    {
        Load();
    }

    void Load()
    {
        string path = "$profile:\\ExpansionTerritoryFuel.json";
        Display = new ETFDisplayCfg();
        Formatting = new ETFFormattingCfg();

        if (!FileExist(path))
        {
            string modPath = "@ExpansionTerritoryFuel/ExpansionTerritoryFuel.json";
            if (!FileExist(modPath))
            {
                modPath = "@ExpansionTerritoryFuel/Addons/ExpansionTerritoryFuel/profile/ExpansionTerritoryFuel.json";
            }

            if (FileExist(modPath))
            {
                JsonFileLoader<ETF_Config>.JsonLoadFile(modPath, this);
            }
        }

        if (FileExist(path))
        {
            JsonFileLoader<ETF_Config>.JsonLoadFile(path, this);
        }
    }
}

class ETFDisplayCfg
{
    bool EnableActionCheck = true;
    bool EnableInventoryTooltip = true;
    string MinRole = "Member";
    int CooldownSeconds = 5;
    float LookAtMaxDistance = 6.0;
}

class ETFFormattingCfg
{
    bool ShowDays = true;
    bool ShowHours = true;
    bool ShowMinutes = true;
    int RoundUpMinutesUnder = 2;
}
