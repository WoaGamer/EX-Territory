#ifdef EXPANSIONMOD
class ActionETFCheckUpkeepCB : ActionSingleUseBaseCB
{
}

class ActionETFCheckUpkeep : ActionSingleUseBase
{
    void ActionETFCheckUpkeep()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
        m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
        m_Text = "#ETF_ACTION_CHECK_UPKEEP";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!target)
            return false;

        EntityAI eai = EntityAI.Cast(target.GetObject());
        if (!eai)
            return false;

        TerritoryFlag flag;
        if (!Class.CastTo(flag, eai))
            return false;

        if (!ETF_Config.Get().Display.EnableActionCheck)
            return false;

        return true;
    }

    override void OnStartClient(ActionData action_data)
    {
        super.OnStartClient(action_data);

        EntityAI flag = EntityAI.Cast(action_data.m_Target.GetObject());
        if (!flag)
            return;

        ETF_Rpc.SendUpkeepRequest(flag, action_data.m_Player, ETFUpkeepRequestOrigin_Action);
    }
}

modded class TerritoryFlag
{
    override void SetActions()
    {
        super.SetActions();

        AddAction(ActionETFCheckUpkeep);
    }
}
#endif
