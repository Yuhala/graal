/*
 * Created on Wed Sep 09 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */
package iiun.smartc;

import java.util.ArrayList;
import java.util.HashMap;

public class Contract {
    private String ledgerHash;
    private HashMap<String, Asset> ledger;
    private String contractId;

    public Contract(String cId) {
        this.contractId = cId;        
        System.out.println("Created contract: " + cId);
    }

    /**
     * Initialize ledger with dummy assets
     */
    public void initLedger() {

        // create Asset objects
        Asset asset1 = createAsset("asset1", "peer1", 100);
        Asset asset2 = createAsset("asset2", "peer1", 200);
        Asset asset3 = createAsset("asset3", "peer2", 150);
        Asset asset4 = createAsset("asset4", "peer2", 250);

        // add Asset objects in ledger
        ledger = new HashMap<String, Asset>();
        ledger.put("asset1", asset1);
        ledger.put("asset2", asset2);
        ledger.put("asset3", asset3);
        ledger.put("asset4", asset4);

        System.out.println("Initialized ledger with dummy assets");

    }

    public Asset createAsset(String assetId, String peerId, int price) {
        return (new Asset(assetId, peerId, price));
    }

    public void getAsset(String aId) {
        // pring all asset info
        // TODO
        System.out.println("Asset name: " + aId);
    }

    public void transferAsset(String assetId, String buyerId, String sellerId) {
        // modify owners in ledger
        // TODO
        System.out.println("Transfered asset: " + assetId + " from: " + sellerId + " to: " + buyerId);
    }
}
