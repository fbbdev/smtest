#include "smlisp.h"

#include <stdio.h>

int main() {
    SmContext* ctx = sm_context((SmGCConfig){ 64, 2, 64 });
    sm_register_builtins(ctx);

    SmValue* form = sm_heap_root(&ctx->heap);
    sm_build_list(ctx, form,
        SmBuildCar, sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("print"))),
        SmBuildList,
            SmBuildCar, sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("+"))),
            SmBuildCar, sm_value_number(sm_number_int(1)),
            SmBuildCar, sm_value_number(sm_number_int(2)),
            SmBuildList,
                SmBuildCar, sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("*"))),
                SmBuildCar, sm_value_number(sm_number_int(3)),
                SmBuildCar, sm_value_number(sm_number_int(4)),
                SmBuildEnd,
            SmBuildList,
                SmBuildCar, sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("-"))),
                SmBuildCar, sm_value_number(sm_number_int(6)),
                SmBuildEnd,
            SmBuildEnd,
        SmBuildEnd);

    printf("evaluating: ");
    sm_print_value(stdout, *form);
    printf("\n");

    SmValue* res = sm_heap_root(&ctx->heap);
    SmError err = sm_eval(ctx, *form, res);

    if (err.code != SmErrorOk) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    printf("evaluating: ");
    *form = sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("print")));
    sm_print_value(stdout, *form);
    printf("\n");

    err = sm_eval(ctx, *form, res);

    if (err.code != SmErrorOk) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    sm_heap_root_drop(&ctx->heap, ctx->frame, form);
    sm_heap_root_drop(&ctx->heap, ctx->frame, res);

    sm_context_drop(ctx);
    return 0;
}
