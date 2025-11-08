#ifdef EXPANSIONMOD
enum ETF_RpcId
{
    RPC_ETF_UPKEEP_REQUEST = 65210,
    RPC_ETF_UPKEEP_REPLY   = 65211
};

enum ETFUpkeepRequestOrigin
{
    ETFUpkeepRequestOrigin_None,
    ETFUpkeepRequestOrigin_Action,
    ETFUpkeepRequestOrigin_Tooltip
};

class ETF_Rpc
{
    static void SendUpkeepRequest(EntityAI flag, PlayerBase player, ETFUpkeepRequestOrigin origin = ETFUpkeepRequestOrigin_Action)
    {
        if (!flag || !player)
            return;

        ETF_ClientRequestState.Get().RegisterRequest(flag, origin);

        ScriptRPC rpc = new ScriptRPC();
        int idLow, idHigh;
        flag.GetNetworkID(idLow, idHigh);
        rpc.Write(idLow);
        rpc.Write(idHigh);
        rpc.Send(null, RPC_ETF_UPKEEP_REQUEST, true, player.GetIdentity());
    }

    static void SendUpkeepReply(PlayerIdentity ident, float hoursLeft, int tokensLeft, int nextConsumeEpoch, string formatted)
    {
        if (!ident)
            return;

        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(hoursLeft);
        rpc.Write(tokensLeft);
        rpc.Write(nextConsumeEpoch);
        rpc.Write(formatted);
        rpc.Send(null, RPC_ETF_UPKEEP_REPLY, true, ident);
    }
}

class ETF_ClientRequestState
{
    protected static ref ETF_ClientRequestState s_Instance;

    protected int m_FlagIdLow;
    protected int m_FlagIdHigh;
    protected ETFUpkeepRequestOrigin m_Origin = ETFUpkeepRequestOrigin_None;

    static ETF_ClientRequestState Get()
    {
        if (!s_Instance)
            s_Instance = new ETF_ClientRequestState();
        return s_Instance;
    }

    void RegisterRequest(EntityAI flag, ETFUpkeepRequestOrigin origin)
    {
        m_Origin = origin;
        m_FlagIdLow = 0;
        m_FlagIdHigh = 0;
        if (!flag)
            return;
        flag.GetNetworkID(m_FlagIdLow, m_FlagIdHigh);
        ETF_InventoryTooltipHelper.HandleClientRequest(flag, origin);
    }

    void HandleReply(float hoursLeft, int tokensLeft, int nextConsumeEpoch, string formatted)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            if (m_Origin == ETFUpkeepRequestOrigin_Action)
            {
                ExpansionNotification("#ETF_NOTIFY_TITLE", string.Format("#ETF_NOTIFY_RESULT", formatted), "set:expansion_notification_iconset image:icon_info").Create(player.GetIdentity());
            }
        }

        ETF_InventoryTooltipHelper.ApplyResult(m_FlagIdLow, m_FlagIdHigh, formatted, m_Origin);

        m_Origin = ETFUpkeepRequestOrigin_None;
        m_FlagIdLow = 0;
        m_FlagIdHigh = 0;
    }
}

modded class MissionServer
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, target, rpc_type, ctx);

        if (rpc_type == RPC_ETF_UPKEEP_REQUEST)
        {
            int idLow;
            int idHigh;
            if (!ctx.Read(idLow))
                return;
            if (!ctx.Read(idHigh))
                return;

            EntityAI flag = EntityAI.Cast(GetGame().GetObjectByNetworkId(idLow, idHigh));
            PlayerBase pb = PlayerBase.Cast(GetGame().GetPlayerByIdentity(sender));
            if (!flag || !pb)
                return;

            TerritoryFlag territoryFlag;
            if (!Class.CastTo(territoryFlag, flag))
                return;

            ETF_FlagComponent comp = territoryFlag.ETF_GetComponent();
            if (!comp)
                return;

            ETFDisplayCfg disp = ETF_Config.Get().Display;

            string deny;
            int wait;
            if (!comp.CanPlayerQuery(pb, disp.LookAtMaxDistance, deny, wait))
            {
                if (deny == "COOLDOWN")
                {
                    string waitMsg = string.Format("#ETF_NOTIFY_COOLDOWN", wait.ToString());
                    ExpansionNotification("#ETF_NOTIFY_TITLE", waitMsg, "set:expansion_notification_iconset image:icon_info").Create(sender);
                }
                else if (deny != "")
                {
                    ExpansionNotification("#ETF_NOTIFY_TITLE", "#" + deny, "set:expansion_notification_iconset image:icon_info").Create(sender);
                }
                return;
            }

            float hoursLeft;
            int tokensLeft;
            int nextConsumeEpoch;
            comp.ComputeUpkeep(hoursLeft, tokensLeft, nextConsumeEpoch);

            string formatted = ETF_UIHelpers.FormatDurationHours(hoursLeft, ETF_Config.Get().Formatting);
            ETF_Rpc.SendUpkeepReply(sender, hoursLeft, tokensLeft, nextConsumeEpoch, formatted);
        }
    }
}

modded class MissionGameplay
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, target, rpc_type, ctx);

        if (rpc_type == RPC_ETF_UPKEEP_REPLY)
        {
            float hoursLeft;
            int tokensLeft;
            int nextConsumeEpoch;
            string formatted;

            if (!ctx.Read(hoursLeft))
                return;
            if (!ctx.Read(tokensLeft))
                return;
            if (!ctx.Read(nextConsumeEpoch))
                return;
            if (!ctx.Read(formatted))
                return;

            ETF_ClientRequestState.Get().HandleReply(hoursLeft, tokensLeft, nextConsumeEpoch, formatted);
        }
    }
}
#endif
