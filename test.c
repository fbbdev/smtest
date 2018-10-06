#include "smlisp.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    SmContext* ctx = sm_context((SmGCConfig){ 64, 2, 64 });
    sm_register_builtins(ctx);

    SmStackFrame frame;
    sm_context_enter_frame(ctx, &frame, sm_string_from_cstring("test"), sm_value_nil());

    SmValue* form = sm_heap_root(&ctx->heap);

    // Test garbage collection for strings
    char* str = sm_heap_alloc_string(&ctx->heap, ctx->frame, 10);
    *form = sm_value_string((SmString){ str, 10 }, str);
    memcpy(str, "ciao bello", sizeof(char)*10);

    printf("string: ");
    sm_print_value(stdout, *form);

    printf("\nobject count before collection: %zu\n", ctx->heap.gc.object_count);
    sm_heap_gc(&ctx->heap, ctx->frame);
    printf("- after first collection: %zu\n", ctx->heap.gc.object_count);
    *form = sm_value_nil();
    sm_heap_gc(&ctx->heap, ctx->frame);
    printf("- after second collection: %zu\n", ctx->heap.gc.object_count);

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

    if (!sm_is_ok(err)) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    *form = sm_value_word(sm_word(&ctx->words, sm_string_from_cstring("print")));
    printf("evaluating: ");
    sm_print_value(stdout, *form);
    printf("\n");

    err = sm_eval(ctx, *form, res);

    if (!sm_is_ok(err)) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    *form = *res;
    printf("evaluating: ");
    sm_print_value(stdout, *form);
    printf("\n");

    err = sm_eval(ctx, *form, res);

    if (!sm_is_ok(err)) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    printf("invoking lambda\n");
    SmCons cons = { sm_value_number(sm_number_int(64)), sm_value_nil() };
    err = sm_invoke_lambda(ctx, form->data.cons->cdr.data.cons, sm_value_cons(&cons), res);

    if (!sm_is_ok(err)) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    if (argc > 1)
        printf("parsing and evaluating arguments\n");

    for (int i = 1; i < argc; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "argv[%d]", i);

        SmParser parser = sm_parser(sm_string_from_cstring(name), sm_string_from_cstring(argv[i]));
        err = sm_parser_parse_all(&parser, ctx, form);
        if (!sm_is_ok(err)) {
            sm_report_error(stdout, err);
            continue;
        }

        *res = sm_value_nil();

        for (SmCons* cons = form->data.cons; cons; cons = sm_list_next(cons)) {
            printf("evaluating: ");
            sm_print_value(stdout, cons->car);
            printf("\n");

            *res = sm_value_nil();
            err = sm_eval(ctx, cons->car, res);
            if (!sm_is_ok(err)) {
                sm_report_error(stdout, err);
                break;
            }
        }

        if (sm_is_ok(err)) {
            printf("result: ");
            sm_print_value(stdout, *res);
            printf("\n");
        }
    }

    sm_heap_root_drop(&ctx->heap, ctx->frame, form);
    sm_heap_root_drop(&ctx->heap, ctx->frame, res);

    sm_context_exit_frame(ctx);
    sm_context_drop(ctx);
    return 0;
}
