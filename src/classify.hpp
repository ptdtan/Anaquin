#ifndef GI_CLASSIFY_HPP
#define GI_CLASSIFY_HPP

#include "standard.hpp"
#include <ss/ml/classify.hpp>

namespace Spike
{
    struct Confusion : public SS::Confusion
    {
        /*
         * The usual formula: tn / (tn + fp) would not work. We don't know
         * tn, furthermore fp would have been dominated by tn. The formula
         * below is consistent to cufflink's recommendation. Technically,
         * we're not calculating specificity but positive predication value.
         */

        inline Percentage sp() const
        {
            return ((tp() + fp()) && fp() != n()) ? tp() / (tp() + fp()) : NAN;
        }
    };
    
    inline bool tfp(bool cond, Confusion *m1, Confusion *m2 = NULL)
    {
        if (cond)
        {
            if (m1) { m1->tp()++; }
            if (m2) { m2->tp()++; }
        }
        else
        {
            if (m1) { m1->fp()++; }
            if (m2) { m2->fp()++; }
        }

        return cond;
    }

    template <typename Iter, typename T> bool find(const Iter &iter, const T &t)
    {
        for (auto i: iter)
        {
            if (i.l.contains(t.l))
            {
                return true;
            }
        }

        return false;
    }

    /*
     * Specalised classification for the project. Negativity isn't required because in a typical
     * experiment the dilution would be so low that negativity dominates positivity.
     */

    template <typename T, typename Stats, typename Positive>
    void classify(Stats &stats, const T &t, Positive p)
    {
        static const Standard &r = Standard::instance();

        SS::classify(stats.mb, t,
            [&](const T &)  // Classifier
            {
                return (t.id == r.id && r.l.contains(t.l));
            },
            [&](const T &t) // Positive?
            {
                stats.nr++;
                return p(t);
            },
            [&](const T &t) // Negative?
            {
                stats.nq++;
                return false;
            });
    }
}

#endif