class ETF_UIHelpers
{
    static string FormatDurationHours(double hours, ETFFormattingCfg f)
    {
        if (!f)
            f = ETF_Config.Get().Formatting;

        int totalMinutes = Math.Round(hours * 60.0);
        if (f && f.RoundUpMinutesUnder > 0 && totalMinutes > 0 && totalMinutes < f.RoundUpMinutesUnder)
        {
            totalMinutes = f.RoundUpMinutesUnder;
        }

        int minutesPerDay = 60 * 24;
        int days = totalMinutes / minutesPerDay;
        int rem = totalMinutes % minutesPerDay;
        int hrs = rem / 60;
        int mins = rem % 60;

        TStringArray parts = {};
        if (f && f.ShowDays && days > 0)
        {
            parts.Insert(days.ToString() + "d");
        }
        if (f && f.ShowHours && (hrs > 0 || days > 0))
        {
            parts.Insert(hrs.ToString() + "h");
        }
        if (f && f.ShowMinutes && mins > 0)
        {
            parts.Insert(mins.ToString() + "m");
        }

        if (parts.Count() == 0)
        {
            return "0m";
        }

        return parts.Join(" ");
    }
}
