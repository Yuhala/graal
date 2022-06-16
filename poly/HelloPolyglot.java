
import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.CurrentIsolate;

import java.nio.file.Files;
import java.nio.file.Path;

public class HelloPolyglot {

    public static Context ctx = Context.create();

    public static void main(String[] args) {
        System.out.println("Hello Java!");
        // evaluate JS code
        //evalJSCode();

          // Testing centrypoints
          System.out.println("Creating isolate and Testing entrypoint");
          IsolateThread iso = CurrentIsolate.getCurrentThread();
          int sum = polytaint_add(iso, 23, 27);
          System.out.println("Sum from isolate entrypoint is: " + sum);

        // evaluate JS source
        /* String js_src_string = "function hello() {\n" +
                "print('Hello JavaScript! xxxxxx +++++ -----');\n" +
                "}\n" +
                "hello();\n"; */
        //evalJsSource(js_src_string);
        //String js_file_src = readFileContent("./hello.js");
        //evalJsSource(js_file_src);

    }

    /**
     * GraalVM entry point test
     */
    @CEntryPoint(name = "polytaint_add")
    static int polytaint_add(IsolateThread thread, int a, int b) {
        return a + b;
    }

    /**
     * Evaluate secureL code
     */
    static void evalSecLCode() {
        // run secureL code
        int secureInt = ctx.eval("secL", "sInt(25)").asInt();
        // int secureInt = ctx.eval("secL", "").asInt();
        System.out.println("My int in SecL is: " + secureInt);
    }

    static String readFileContent(String fileName) {
        String content = "";
        try {
            content = Files.readString(Path.of(fileName));
        } catch (Exception e) {
            // TODO: handle exception
        }
        return content;
    }

    /**
     * Evaluate JS code
     */
    static void evalJSCode() {
        // run javascript code
        ctx.eval("js", "print('Hello javascript!');");
        Value array = ctx.eval("js", "[1,2,42,4]");

        int result = array.getArrayElement(2).asInt();
        System.out.println("Array[2]: " + result);

        int myInt = ctx.eval("js", "2").asInt();
        System.out.println("My int in JS is: " + myInt);
    }

    static void evalJsSource(String src_string) {
        System.out.println("++++++++++++ Evaluating JS File +++++++++++++++++");
        String function_obj = "(function(){" + src_string + "})\n";

        // Source js_src = Source.newBuilder("js", src_string, "mul.js").buildLiteral();
        Value val = ctx.eval("js", src_string);

    }

    static void contextWithTry() {
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "print('Hello JavaScript!');");

        }

    }
}

/*
 * #include <stdio.h>
 * #include <graalvm/llvm/polyglot.h>
 * 
 * int main() {
 * void *array = polyglot_eval("js", "[1,2,42,4]");
 * int element = polyglot_as_i32(polyglot_get_array_element(array, 2));
 * printf("%d\n", element);
 * return element;
 * }
 */