#include "smlisp.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    SmContext* ctx = sm_context((SmGCConfig){ 64, 2, 64 });
    sm_register_builtins(ctx);

    SmStackFrame frame;
    sm_context_enter_frame(ctx, &frame, sm_string_from_cstring("test"));

    SmValue* form = sm_heap_root_value(&ctx->heap);

    // Test garbage collection for strings
    char* str = sm_heap_alloc_string(&ctx->heap, ctx, 10);
    *form = sm_value_string((SmString){ str, 10 }, str);
    memcpy(str, "ciao bello", sizeof(char)*10);

    printf("string test: ");
    sm_print_value(stdout, *form);

    printf("\nobject count before collection: %zu\n", ctx->heap.gc.object_count);
    sm_heap_gc(&ctx->heap, ctx);
    printf("- after first collection: %zu\n", ctx->heap.gc.object_count);
    *form = sm_value_nil();
    sm_heap_gc(&ctx->heap, ctx);
    printf("- after second collection: %zu\n", ctx->heap.gc.object_count);

    printf("\nscope test: ");

    SmScope** scope = sm_heap_root_scope(&ctx->heap);
    *scope = sm_heap_alloc_scope(&ctx->heap, ctx);

    sm_scope_set(*scope, (SmSymbol) 0, sm_value_cons(sm_heap_alloc_cons(&ctx->heap, ctx)));
    sm_scope_set(*scope, (SmSymbol) 1, sm_value_cons(sm_heap_alloc_cons(&ctx->heap, ctx)));

    printf("%zu\n", sm_scope_size(*scope));

    printf("variable 0: ");
    sm_print_value(stdout, sm_scope_get(*scope, (SmSymbol) 0)->value);
    printf("\nvariable 1: ");
    sm_print_value(stdout, sm_scope_get(*scope, (SmSymbol) 1)->value);

    printf("\nobject count before collection: %zu\n", ctx->heap.gc.object_count);

    sm_heap_gc(&ctx->heap, ctx);
    printf("- after first collection: %zu\n", ctx->heap.gc.object_count);

    printf("variable 0: ");
    sm_print_value(stdout, sm_scope_get(*scope, (SmSymbol) 0)->value);
    printf("\nvariable 1: ");
    sm_print_value(stdout, sm_scope_get(*scope, (SmSymbol) 1)->value);


    sm_scope_delete(*scope, (SmSymbol) 0);
    sm_heap_gc(&ctx->heap, ctx);
    printf("\n- after second collection: %zu\n", ctx->heap.gc.object_count);

    printf("variable 1: ");
    sm_print_value(stdout, sm_scope_get(*scope, (SmSymbol) 1)->value);

    sm_heap_root_scope_drop(&ctx->heap, ctx, scope);
    sm_heap_gc(&ctx->heap, ctx);
    printf("\n- after third collection: %zu\n\n", ctx->heap.gc.object_count);

    sm_build_list(ctx, form,
        SmBuildCar, sm_value_symbol(sm_symbol(&ctx->symbols, sm_string_from_cstring("print"))),
        SmBuildList,
            SmBuildCar, sm_value_symbol(sm_symbol(&ctx->symbols, sm_string_from_cstring("+"))),
            SmBuildCar, sm_value_number(sm_number_int(1)),
            SmBuildCar, sm_value_number(sm_number_int(2)),
            SmBuildList,
                SmBuildCar, sm_value_symbol(sm_symbol(&ctx->symbols, sm_string_from_cstring("*"))),
                SmBuildCar, sm_value_number(sm_number_int(3)),
                SmBuildCar, sm_value_number(sm_number_int(4)),
                SmBuildEnd,
            SmBuildList,
                SmBuildCar, sm_value_symbol(sm_symbol(&ctx->symbols, sm_string_from_cstring("-"))),
                SmBuildCar, sm_value_number(sm_number_int(6)),
                SmBuildEnd,
            SmBuildEnd,
        SmBuildEnd);

    printf("evaluating: ");
    sm_print_value(stdout, *form);
    printf("\n");

    SmValue* res = sm_heap_root_value(&ctx->heap);
    SmError err = sm_eval(ctx, *form, res);

    if (!sm_is_ok(err)) {
        sm_report_error(stdout, err);
    } else {
        printf("result: ");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    *form = sm_value_symbol(sm_symbol(&ctx->symbols, sm_string_from_cstring("print")));
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
        printf("result (function: %s): ", sm_value_is_function(*res) ? "true" : "false");
        sm_print_value(stdout, *res);
        printf("\n");
    }

    printf("invoking function: %.*s\n",
        (int) form->data.function->args.name.length, form->data.function->args.name.data);
    SmCons cons = { sm_value_number(sm_number_int(64)), sm_value_nil() };
    err = sm_function_invoke(form->data.function, ctx, sm_value_cons(&cons), res);

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

    sm_heap_root_value_drop(&ctx->heap, ctx, form);
    sm_heap_root_value_drop(&ctx->heap, ctx, res);

    sm_context_exit_frame(ctx);
    sm_context_drop(ctx);
    return 0;
}
