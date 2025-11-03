#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#include "utils.h"

enum klas_van_tipe {
	kvt_basies,
	kvt_verwysing,
	kvt_vaste_lys,
	kvt_dinamiese_lys,
	kvt_funksie,
};
struct tipe;
enum tp_basiese_tipe {
	// die kern tipes van die taal, "primitive types" in Engels
	bt_niks, // "niks" tipe vir as 'n funksie niks terugkeer nie
	         // sy grote is nul, en daar kan nie 'n eintlike waarde van hierdie tipe bestaan nie
	         // dit word ook net in sekere kontekste toegelaat
	         // (kan bv nie 'n veranderlike van tipe "niks" hê nie)
	bt_karakter,
	bt_teks, // ekwivalent aan 'n vaste/dinamiese lys van karakters
	bt_bool, // waar of vals
	bt_nat8, // positiewe 8-bis heelgetal (natuurlike getal)
	bt_heel8, // positiewe of negatiewe 8-bis heelgetal
	bt_nat16,
	bt_heel16,
	bt_nat32,
	bt_heel32,
	bt_nat64,
	bt_heel64,
	bt_nat_masjien, // masjiengrote: 64-bis vir nou
	bt_heel_masjien,
};
DECL_DISPLAY_METHS_ENUM(tp_basiese_tipe);
static const char *tp_basiese_tipe_str[] = {
	[bt_niks] = "niks",
	[bt_karakter] = "karakter",
	[bt_teks] = "teks",
	[bt_bool] = "bool",
	[bt_nat8] = "nat8",
	[bt_heel8] = "heel8",
	[bt_nat16] = "nat16",
	[bt_heel16] = "heel16",
	[bt_nat32] = "nat32",
	[bt_heel32] = "heel32",
	[bt_nat64] = "nat64",
	[bt_heel64] = "heel64",
	[bt_nat_masjien] = "nat_masjien",
	[bt_heel_masjien] = "heel_masjien",
};
struct tp_verwysing {
	// "pointer" in Engels (alhowel "reference" dalk 'n meer akkurate terugvertaling sal wees)
	// word voorgestel in die geheue as dieselfde as bt_nat_masjien
	struct tipe *na;
};
DECL_STD_METHS(tp_verwysing);
struct tp_vaste_lys {
	// 'n lys met 'n sekere, vaste lengte van 'n sekere tipe
	struct tipe *van;
	size_t lengte;
};
DECL_STD_METHS(tp_vaste_lys);
struct tp_dinamiese_lys {
	// 'n lys met 'n dinamiese lengte,
	// dws die lengte kan verander word terwyl die program hardloop,
	// dus is die lengte nie bekend aan die compiler nie
	struct tipe *van;
};
DECL_STD_METHS(tp_dinamiese_lys);
struct tp_argumente {
	// die argumentelys van 'n funksie, die name van die argumente maak nie 'n verskil aan die tipe nie
	// 'n "dynamic array", gemaak om met nob.h te werk
	struct tipe *items;
	size_t count;
	size_t capacity;
};
DECL_STD_METHS(tp_argumente);
struct tp_funksie {
	// 'n funksie; vir die tipe maak net die terugkeertipe en die tipes van die argumente saak
	// argumentname en die funksie se naam maak nie saam nie
	// as 'n eintlike waarde, dink ek 'n funksie is 'n lokasie in die rekenaar se geheue,
	// maar ook nie heeltemal nie, meer 'n "label".
	// Dus sal die groote van 'n funksie waarde "0" wees.
	struct tipe *terugkeer;
	struct tp_argumente argumente;
};
DECL_STD_METHS(tp_funksie);
struct tipe {
	enum klas_van_tipe klas;
	union {
		enum tp_basiese_tipe basies;
		struct tp_verwysing verwysing;
		struct tp_vaste_lys vaste_lys;
		struct tp_dinamiese_lys dinamiese_lys;
		struct tp_funksie funksie;
	};
};
DECL_STD_METHS(tipe);

#endif /* TYPES_H */
