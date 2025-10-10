@sluit_in stdiu

// ekwivalent aan `funk fib(n: kon heel) -> ver heel`
funk fib(n: heel) -> heel {
	// `[2]heel` sal `[2]kon heel` wees
	laaste_twee: [2]ver heel = [ 0, 1 ];

	// ekwivalent aan `vir i: ver heel = 1; ...`
	vir i: heel = 1; i <= n; ++i {
		// ekwivalent aan `som: kon heel`
		som: heel = laaste_twee[0] + laaste_twee[1];
		laaste_twee[0] = laaste_twee[1];
		laaste_twee[1] = som;
	};

	laaste_twee[0]
};

// ekwivalent aan `funk fib2(n: kon heel) -> rusultaat: ver heel`
funk fib2(n: heel) -> rusultaat: heel {
	volgende: ver heel = 1;
	rusultaat = 0;

	vir i: heel = 1; i <= n; ++i {
		som: heel = rusultaat + volgende;
		rusultaat = volgende;
		volgende = som;
	};
};

funk hoof() {
	druk("Gee 'n nommer: ");
	n: heel = lees_heel();
	druk("Die ", n, "ste Fibonacci nommer is ", fib(n));
};
