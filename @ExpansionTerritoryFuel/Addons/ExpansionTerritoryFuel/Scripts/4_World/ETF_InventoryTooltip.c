#ifdef EXPANSIONMOD
class ETF_TooltipBinding
{
    ContainerWithCargoAndAttachments m_Container;
    string m_Key;

    void ETF_TooltipBinding(ContainerWithCargoAndAttachments container, string key)
    {
        m_Container = container;
        m_Key = key;
    }
}

class ETF_InventoryTooltipHelper
{
    protected static ref ETF_InventoryTooltipHelper s_Instance;

    protected ref map<string, string> m_Hints = new map<string, string>();
    protected ref array<ref ETF_TooltipBinding> m_Bindings = new array<ref ETF_TooltipBinding>();

    static ETF_InventoryTooltipHelper Get()
    {
        if (!s_Instance)
            s_Instance = new ETF_InventoryTooltipHelper();
        return s_Instance;
    }

    static void HandleClientRequest(EntityAI flag, ETFUpkeepRequestOrigin origin)
    {
        if (origin != ETFUpkeepRequestOrigin_Tooltip)
            return;

        if (!flag)
            return;

        int idLow;
        int idHigh;
        flag.GetNetworkID(idLow, idHigh);

        string key = MakeKey(idLow, idHigh);
        Get().m_Hints.Set(key, "#ETF_TOOLTIP_LOADING");
        Get().UpdateBindings(key, "#ETF_TOOLTIP_LOADING");
    }

    static void ApplyResult(int idLow, int idHigh, string formatted, ETFUpkeepRequestOrigin origin)
    {
        if (idLow == 0 && idHigh == 0)
            return;

        string key = MakeKey(idLow, idHigh);
        if (formatted != "")
            Get().m_Hints.Set(key, formatted);
        else
            Get().m_Hints.Remove(key);

        Get().UpdateBindings(key, formatted);
    }

    static void RegisterContainer(ContainerWithCargoAndAttachments container, EntityAI flag)
    {
        if (!container || !flag)
            return;

        int idLow;
        int idHigh;
        flag.GetNetworkID(idLow, idHigh);
        string key = MakeKey(idLow, idHigh);

        string hint;
        if (!Get().m_Hints.Find(key, hint))
        {
            hint = "#ETF_TOOLTIP_LOADING";
            Get().m_Hints.Set(key, hint);
        }

        container.m_ETF_FlagKey = key;
        container.ETF_UpdateHint(hint);
        Get().CleanupBindings(container);
        Get().m_Bindings.Insert(new ETF_TooltipBinding(container, key));
    }

    static void UnregisterContainer(ContainerWithCargoAndAttachments container)
    {
        if (!container)
            return;

        Get().CleanupBindings(container);
    }

    static bool QueryHint(string key, out string hint)
    {
        return Get().m_Hints.Find(key, hint);
    }

    protected static string MakeKey(int idLow, int idHigh)
    {
        return string.Format("%1:%2", idLow, idHigh);
    }

    protected void CleanupBindings(ContainerWithCargoAndAttachments container = null)
    {
        for (int i = m_Bindings.Count() - 1; i >= 0; i--)
        {
            ETF_TooltipBinding binding = m_Bindings[i];
            if (!binding || !binding.m_Container)
            {
                m_Bindings.Remove(i);
                continue;
            }

            if (container && binding.m_Container == container)
            {
                m_Bindings.Remove(i);
                continue;
            }
        }
    }

    protected void UpdateBindings(string key, string formatted)
    {
        for (int i = m_Bindings.Count() - 1; i >= 0; i--)
        {
            ETF_TooltipBinding binding = m_Bindings[i];
            if (!binding || !binding.m_Container)
            {
                m_Bindings.Remove(i);
                continue;
            }

            if (binding.m_Key == key)
            {
                binding.m_Container.ETF_UpdateHint(formatted);
            }
        }
    }
}

modded class ContainerWithCargoAndAttachments
{
    string m_ETF_UpkeepHint;
    string m_ETF_FlagKey;

    override void SetEntity(EntityAI entity)
    {
        super.SetEntity(entity);

        if (GetGame().IsDedicatedServer())
            return;

        if (!ETF_Config.Get().Display.EnableInventoryTooltip)
            return;

        TerritoryFlag flag;
        if (Class.CastTo(flag, entity))
        {
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                ETF_InventoryTooltipHelper.RegisterContainer(this, flag);
                ETF_Rpc.SendUpkeepRequest(flag, player, ETFUpkeepRequestOrigin_Tooltip);
            }
        }
        else
        {
            ETF_InventoryTooltipHelper.UnregisterContainer(this);
        }
    }

    override bool GetTooltipText(out string text)
    {
        bool hasSuper = super.GetTooltipText(text);

        if (m_ETF_FlagKey != "")
        {
            string hint = m_ETF_UpkeepHint;
            if (hint == "")
                ETF_InventoryTooltipHelper.QueryHint(m_ETF_FlagKey, hint);

            if (hint != "")
            {
                string formatted = string.Format("#ETF_TOOLTIP_UPKEEP", hint);
                if (hasSuper)
                    text = text + "\n" + formatted;
                else
                    text = formatted;
                return true;
            }
        }

        return hasSuper;
    }

    void ETF_UpdateHint(string hint)
    {
        m_ETF_UpkeepHint = hint;
    }

    void ~ContainerWithCargoAndAttachments()
    {
        ETF_InventoryTooltipHelper.UnregisterContainer(this);
    }
}
#endif
