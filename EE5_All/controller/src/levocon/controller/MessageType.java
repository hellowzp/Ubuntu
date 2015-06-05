package levocon.controller;

public enum MessageType {
	ADVERTISE,
	SEARCHGW,
	GWINFO,
	RESERVED_3,
	CONNECT,
	CONNACK,
	WILLTOPICREQ,
	WILLTOPIC,
	WILLMSGREQ,
	WILLMSG,
	REGISTER,
	REGACK,
	PUBLISH,
	PUBACK,
	PUBCOMP,
	PUBREC,
	PUBREL,
	RESERVED_17,
	SUBSCRIBE,
	SUBACK,
	UNSUBSCRIBE,
	UNSUBACK,
	PINGREQ,
	PINGRESP,
	DISCONNECT,
	RESERVED_25,
	WILLTOPICUPD,
	WILLTOPICRESP,
	WILLMSGUPD,
	WILLMSGRESP
}