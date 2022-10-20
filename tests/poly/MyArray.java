
package poly;

import org.graalvm.polyglot.*;
import org.graalvm.nativeimage.SGXSerializer;
import java.util.ArrayList;

public class MyArray {
    public int[] array;
    ArrayList<ArrayList<Double>> myArrList = new ArrayList<ArrayList<Double>>();

    public MyArray(int n) {
        this.array = new int[n];
    }

    public void setArray(int[] a) {
        this.array = a;
    }
}
