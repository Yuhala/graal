/*
 * Created on Wed Sep 09 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

package iiun.smartc;


public class Peer {
    private String peerId;
    private String ledgerHash;

    public Peer(String peerId) {
		this.peerId = peerId;
		this.ledgerHash ="xxx";
		System.out.println("Created peer: "+peerId);
	}

    public String getPeerId() {
        return peerId;
    }

    public static void helloJava(){
        System.out.println("----- Hello from Java -----::)");
    }

    public void setPeerId(String pid) {
        this.peerId = pid;
    }

    public String getLedgerHash() {
        return ledgerHash;
    }

    public void setLedgerhash(String hash) {
        this.ledgerHash = hash;
    }
}
