package com.android.bshadow;

import java.util.UUID;

public class BleDefinedUUIDs {
	
	public static class Service {
		final static public UUID QPPS               = UUID.fromString("0000fee9-0000-1000-8000-00805f9b34fb");
	};

	public static class Characteristic {
		final static public UUID NS1 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129601");
		final static public UUID NS2 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129602");
		final static public UUID NS3 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129603");
		final static public UUID NS4 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129604");
		final static public UUID NS5 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129605");
		final static public UUID NS6 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129606");
		final static public UUID WS1 = UUID.fromString("d44bc439-abfd-45a2-b575-925416129600");
	}
}
