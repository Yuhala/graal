/*
 * Created on Wed Sep 09 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

package iiun.smartc;

import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

public class Params {
    public int param1;
    public int param2;

    public Params(int p1, int p2) {
		this.param1 = p1;
        this.param2= p2;
	}

}
