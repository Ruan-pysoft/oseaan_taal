@sluit_in stdiu

funk foo_heel(n: heel) -> heel {
	gee n+1;
};
funk foo_dryf(n: dryf) -> dryf {
	gee n/2;
};
funk foo_teks(n: teks) -> teks {
	ver antw: teks = n:teks_kopie();
	antw:teks_aanlas(n);
	gee antw;
};

uitbreiding foo(n) {
	@as @tipevan(n) == heel { foo_heel(n) }
	@anders @as @tipevan(n) == dryf { foo_dryf(n) }
	@anders @as @tipevan(n) == teks { foo_teks(n) }
	@anders {
		fout("onbekende tipe");
	}
};

spesialisasie(T: tipe)
struk Lys {
	item: T;
	volg: ?*erf struk Lys@(T);
};
spesialisasie(T: tipe)
funk las_aan(l: ?*ver struk Lys@(T), item: T) {
	volg: *struk Lys@(T) = ???;
	volg->item = item;
	volg->volg = niks;

	as is_niks(l) {
		l = volg;
	} anders {
		l->volg = volg;
	}
};

uitbreiding bar = foo;

funk hoof() {
	druk(foo("Haai daar"));
	druk(foo(heel(3.14)));

	ver lys: ?*ver struk Lys@(heel) = niks;
	lys:>las_aan(1);
	lys:>las_aan(2);
	lys:>las_aan(3);
	vir deel: ?*struk Lys@(heel) = lys; nie is_niks(deel); deel = deel->volg {
		druk: bar(deel->item);
	};
};
