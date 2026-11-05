// Same later build order. First Monster follows actual family state/request.
// true means a buy was attempted, including rejection/exception: end this input.
// Root owns crystal-first ordering and invalidation/reobservation after mutation.
#include "qualifier_upgrade_quote.h"
bool qualifier_shop() {
    bool attempted = false;
    try {
        if (!S || !qualifier_current()) return false;
        auto& me = self();
        // Resolve ALL variants from a fresh post-crystal inventory snapshot.
        const auto owned = me.item_count();
        const auto selected = qualifier_shop_selection(owned);
        if (!selected.known || !qualifier_current()) return false;
        const auto quote = qualifier_shop_quote(selected, me.gold_count());
        if (!quote.known || quote.count == 0 || !me.can_buy(quote.item, quote.count) || !qualifier_current()) return false;
        attempted = true;
        me.buy(quote.item, quote.count); // Same selected item/count, no buy falls through.
        return true;
    } catch (...) {
        // No second smaller request and no assumed rollback. Root invalidates
        // old contact/delivery plans on true and reobserves next input.
        return attempted;
    }
}
