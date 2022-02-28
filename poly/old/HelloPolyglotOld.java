
import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

public class HelloPolyglot {
    public static void main(String[] args) {
        System.out.println("Hello Java!");
        // context without try block
        Context ctx = Context.create();

        // run javascript code
        ctx.eval("js", "print('Hello javascript!');");
        Value array = ctx.eval("js", "[1,2,42,4]");

        int result = array.getArrayElement(2).asInt();
        System.out.println("Result is: " + result);

        int myInt = ctx.eval("js", "2").asInt();
        System.out.println("My int in JS is: " + myInt);



        // run simple language code
        ctx.eval("sl", "function main() { println(\"Hello simplelanguage!\");}");
        ctx.eval("sl", "function main() { println(10/3);}");

        int secureInt =   ctx.eval("sl", "function main() {return 50;}").asInt();
        System.out.println("My int in SL is: " + secureInt);  
    }

    static void contextWithTry() {
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "print('Hello JavaScript!');");

        }

    }
}



/* #include <stdio.h>
#include <graalvm/llvm/polyglot.h>

int main() {
    void *array = polyglot_eval("js", "[1,2,42,4]");
    int element = polyglot_as_i32(polyglot_get_array_element(array, 2));
    printf("%d\n", element);
    return element;
} */