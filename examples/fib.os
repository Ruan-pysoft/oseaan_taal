@sluit_in stdiu

funk fib(n: heel) -> heel {
	ver laaste_twee: [2]heel = [ 0, 1 ];

	vir i: heel = 1; i <= heel; ++i {
		kon som: heel = laaste_twee[0] + laaste_twee[1];
		laaste_twee[0] = laaste_twee[1];
		laaste_twee[1] = som;
	};

	laaste_twee[0]
};

funk fib2(n: heel) -> ver rusultaat: heel {
	ver volgende: heel = 1;
	rusultaat = 0;

	vir i: heel = 1; i <= heel; ++i {
		kon som: heel = rusultaat + volgende;
		rusultaat = volgende;
		volgende = som;
	};
};

funk hoof() {
	druk("Gee 'n nommer: ");
	n: heel;
	lees_heel(&n);
	druk("Die ", n, "ste Fibonacci nommer is ", fib(n));
};
