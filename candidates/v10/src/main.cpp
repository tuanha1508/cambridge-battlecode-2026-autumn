#include <cambc.h>
#include "qualifier_draft.h"

int legacy_main();
int optimized_main();

int main() {
    if (cambc::is_draft()) return qualifier_draft::run();
    return cambc::map::width() == 100 && cambc::map::height() == 100
        ? legacy_main() : optimized_main();
}
