laat eksterne druk_nommer: funk(: heel);

laat tel_by: funk(: heel, : heel) -> :heel = funk(a: heel, b: heel) -> :heel {
	a + b
};

laat hoof: funk(:heel) -> :heel;
funk hoof() -> kode: heel {
	kode = 6*7;
	druk_nommer(tel_by(34, 35));
};
