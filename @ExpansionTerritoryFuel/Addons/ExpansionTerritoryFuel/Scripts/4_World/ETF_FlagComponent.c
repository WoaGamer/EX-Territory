#ifdef EXPANSIONMOD
class ETF_FlagComponent
{
    protected TerritoryFlag m_Flag;
    protected float m_NextConsumeAtEpoch;
    protected int m_RemainingTokens;
    protected float m_HoursPerUnit;
    protected ref map<string, int> m_RequestCooldowns = new map<string, int>();

    void ETF_FlagComponent(TerritoryFlag flag)
    {
        m_Flag = flag;
    }

    void OnStoreSave(ParamsWriteContext ctx)
    {
        ctx.Write(m_NextConsumeAtEpoch);
        ctx.Write(m_RemainingTokens);
        ctx.Write(m_HoursPerUnit);
    }

    bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!ctx.Read(m_NextConsumeAtEpoch))
            m_NextConsumeAtEpoch = 0;
        if (!ctx.Read(m_RemainingTokens))
            m_RemainingTokens = 0;
        if (!ctx.Read(m_HoursPerUnit))
            m_HoursPerUnit = 0;
        return true;
    }

    void UpdateState(float nextConsumeAtEpoch, int remainingTokens, float hoursPerUnit)
    {
        m_NextConsumeAtEpoch = nextConsumeAtEpoch;
        m_RemainingTokens = remainingTokens;
        m_HoursPerUnit = hoursPerUnit;
    }

    static void UpdateFlagState(TerritoryFlag flag, float nextConsumeAtEpoch, int remainingTokens, float hoursPerUnit)
    {
        if (!flag)
            return;

        ETF_FlagComponent comp = flag.ETF_GetComponent();
        if (!comp)
            return;

        comp.UpdateState(nextConsumeAtEpoch, remainingTokens, hoursPerUnit);
    }

    void ComputeUpkeep(out float hoursLeft, out int tokensLeft, out int nextConsumeEpoch)
    {
        int nowEpoch = GetGame().GetTime() / 1000;
        tokensLeft = m_RemainingTokens;
        nextConsumeEpoch = (int)m_NextConsumeAtEpoch;

        if (tokensLeft <= 0 || m_HoursPerUnit <= 0.0)
        {
            hoursLeft = 0.0;
            return;
        }

        double firstHours = Math.Max(0.0, (m_NextConsumeAtEpoch - nowEpoch) / 3600.0);
        hoursLeft = firstHours + (tokensLeft - 1) * m_HoursPerUnit;
    }

    bool CanPlayerQuery(PlayerBase pb, float maxDist, out string denyReasonKey, out int waitSeconds)
    {
        denyReasonKey = "";
        waitSeconds = 0;

        if (!pb)
            return false;

        if (!m_Flag)
            return false;

        if (vector.Distance(pb.GetPosition(), m_Flag.GetPosition()) > maxDist)
        {
            denyReasonKey = "ETF_NOTIFY_TOO_FAR";
            return false;
        }

        if (!IsTerritoryMemberOrOwner(pb))
        {
            denyReasonKey = "ETF_NOTIFY_NO_PERMISSION";
            return false;
        }

        ETFDisplayCfg cfg = ETF_Config.Get().Display;
        string uid = pb.GetIdentity() ? pb.GetIdentity().GetId() : "";
        if (uid == "")
            return false;

        int nowEpoch = GetGame().GetTime() / 1000;
        int last;
        if (m_RequestCooldowns.Find(uid, last))
        {
            int delta = nowEpoch - last;
            if (delta < cfg.CooldownSeconds)
            {
                waitSeconds = cfg.CooldownSeconds - delta;
                denyReasonKey = "COOLDOWN";
                return false;
            }
        }

        m_RequestCooldowns.Set(uid, nowEpoch);
        return true;
    }

    protected bool IsTerritoryMemberOrOwner(PlayerBase pb)
    {
        if (!pb || !m_Flag)
            return false;

        PlayerIdentity ident = pb.GetIdentity();
        if (!ident)
            return false;

        string playerId = ident.GetId();
        if (playerId == "")
            return false;

        ExpansionTerritory territory = m_Flag.GetTerritory();
        if (!territory)
            return false;

        ETFDisplayCfg disp = ETF_Config.Get().Display;

        string ownerId = territory.GetOwnerID();
        bool isOwner = ownerId == playerId;
        if (disp.MinRole == "Owner")
            return isOwner;

        if (isOwner)
            return true;

        return territory.IsMember(playerId);
    }
}

modded class TerritoryFlag
{
    protected ref ETF_FlagComponent m_ETF_Component;

    void TerritoryFlag()
    {
        m_ETF_Component = new ETF_FlagComponent(this);
    }

    ETF_FlagComponent ETF_GetComponent()
    {
        return m_ETF_Component;
    }

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        if (m_ETF_Component)
            m_ETF_Component.OnStoreSave(ctx);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;

        if (!m_ETF_Component)
            m_ETF_Component = new ETF_FlagComponent(this);

        if (!m_ETF_Component.OnStoreLoad(ctx, version))
            return false;

        return true;
    }
}
#endif
