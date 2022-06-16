/*
 * Created on Fri Nov 05 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 * Testing polyglot native images
 */

package iiun.smartc;

import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.CurrentIsolate;

public class Main {

    public static Context globalContext;
    public static void main(String[] args) {
        System.out.println("<<< ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hello Java! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>");
        // context without try block

        // test entry points
        // testEntryPoints();

        // test export of java func into js
        // callJavaMethodFromJS();

        // System.out.println("Creating context");
        // Context ctx = Context.create();
        //

        // run javascript code
        System.out.println("<<< ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Building global language context! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>");
        //globalContext = Context.newBuilder().allowAllAccess(true).build();
        Context ctx = Context.create();

        System.out.println("<<< ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Evaluationg js source code! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ >>>");
        ctx.eval("js", "console.log('Hello javascript!');");

        // Value array = ctx.eval("js", "[1,2,42,4]");

        // int result = array.getArrayElement(2).asInt();
        // System.out.println("Result is: " + result);

        // int myInt = ctx.eval("js", "2").asInt();
        // System.out.println("My int in JS is: " + myInt);

        // run secureL code
        // int secureInt = ctx.eval("secL", "sInt(25)").asInt();
        // System.out.println("My int in SecL is: " + secureInt);

    }

    static void contextWithTry() {
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "console.log('Hello JavaScript!');");

        }

    }

    /**
     * GraalVM entry point test
     */
    @CEntryPoint(name = "polytaint_add")
    public static int polytaint_add(IsolateThread thread, int a, int b) {
        return a + b;
    }

    public static void helloJava(int param) {
        System.out.println("----- Hello from Java -----::): param is: " + param);
    }

    public static void helloRuby(int param) {
        System.out.println("----- Hello from Ruby -----::): param is: " + param);
    }

    static void callJavaMethodFromJS() {
        System.out.println("Calling java method from JS context");
        Context ctx = Context.newBuilder().allowAllAccess(true).build();
        Value func1 = ctx.asValue(Main.class).getMember("static").getMember("helloJava");
        Value func2 = ctx.asValue(Main.class).getMember("static").getMember("helloRuby");

        // ctx.eval("js",
        // "function parent_func(m,p){func1 = m.func1;func2 =
        // m.func2;p1=p.param1;p2=p.param2; function
        // real_func(val1,val2){func1(111);func2(222);} real_func(p1,p2);}
        // parent_func;")
        // .execute(new MultiFunction(func1, func2), new Params(1, 2));
        // ctx.eval("js", "function test(f1,f2){f1(111);f2(222);}
        // test;").execute(func1,func2);
    }

    static void testEntryPoints() {
        // Testing centrypoints
        System.out.println("Creating isolate and Testing entrypoint");
        IsolateThread iso = CurrentIsolate.getCurrentThread();
        int sum = polytaint_add(iso, 23, 27);
        System.out.println("Sum from isolate entrypoint is: " + sum);
    }
}
