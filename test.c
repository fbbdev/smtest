#include "smlisp.h"

#include <stdio.h>

int main() {
    SmContext* ctx = sm_context((SmGCConfig){ 64, 2, 64 });
    sm_register_builtins(ctx);

    sm_context_drop(ctx);
    return 0;
}
