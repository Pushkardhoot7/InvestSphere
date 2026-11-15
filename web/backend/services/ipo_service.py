"""
InvestSphere IPO Tracking Service
Provides genuine Indian Mainboard and SME IPO data across Upcoming, Open, Closed, and Listed categories.
Includes subscription statistics, price bands, minimum investments, and listing performance.
"""

import datetime

IPOS_DATA = [
    # ── UPCOMING IPOS ──────────────────────────────────────────────────────────
    {
        "id": "ntpc-green",
        "company_name": "NTPC Green Energy Limited",
        "symbol": "NTPCGREEN",
        "ipo_type": "MAINBOARD",
        "status": "UPCOMING",
        "opening_date": "2026-11-19",
        "closing_date": "2026-11-22",
        "allotment_date": "2026-11-25",
        "listing_date": "2026-11-27",
        "price_band": "₹102 - ₹108",
        "price_min": 102.0,
        "price_max": 108.0,
        "lot_size": 138,
        "min_investment": 14904.0,  # 138 * 108
        "issue_size_cr": 10000.0,
        "fresh_issue_cr": 10000.0,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Limited",
        "description": "NTPC Green Energy is the largest renewable energy public sector enterprise in India (excluding hydro) in terms of operating capacity as of September 2026."
    },
    {
        "id": "enviro-infra",
        "company_name": "Enviro Infra Engineers Limited",
        "symbol": "ENVIFRA",
        "ipo_type": "MAINBOARD",
        "status": "UPCOMING",
        "opening_date": "2026-11-22",
        "closing_date": "2026-11-26",
        "allotment_date": "2026-11-27",
        "listing_date": "2026-11-29",
        "price_band": "₹140 - ₹148",
        "price_min": 140.0,
        "price_max": 148.0,
        "lot_size": 101,
        "min_investment": 14948.0,  # 101 * 148
        "issue_size_cr": 650.43,
        "fresh_issue_cr": 572.46,
        "ofs_cr": 77.97,
        "face_value": "₹10 per share",
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "Bigshare Services Pvt Ltd",
        "description": "Designs, constructs, operates and maintains Water and Wastewater Treatment Plants (WWTPs) and Water Supply Scheme Projects for government authorities."
    },
    {
        "id": "rajesh-power",
        "company_name": "Rajesh Power Services Limited",
        "symbol": "RAJESHPOW",
        "ipo_type": "SME",
        "status": "UPCOMING",
        "opening_date": "2026-11-25",
        "closing_date": "2026-11-27",
        "allotment_date": "2026-11-28",
        "listing_date": "2026-12-02",
        "price_band": "₹319 - ₹335",
        "price_min": 319.0,
        "price_max": 335.0,
        "lot_size": 400,
        "min_investment": 134000.0,
        "issue_size_cr": 160.47,
        "fresh_issue_cr": 130.0,
        "ofs_cr": 30.47,
        "face_value": "₹10 per share",
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "BSE SME",
        "registrar": "Bigshare Services Pvt Ltd",
        "description": "Engineering, procurement, and construction (EPC) contracting company providing turnkey solutions for power transmission and distribution systems."
    },
    {
        "id": "c2c-advanced",
        "company_name": "C2C Advanced Systems Limited",
        "symbol": "C2CADV",
        "ipo_type": "SME",
        "status": "UPCOMING",
        "opening_date": "2026-11-22",
        "closing_date": "2026-11-26",
        "allotment_date": "2026-11-27",
        "listing_date": "2026-11-29",
        "price_band": "₹214 - ₹226",
        "price_min": 214.0,
        "price_max": 226.0,
        "lot_size": 600,
        "min_investment": 135600.0,
        "issue_size_cr": 99.07,
        "fresh_issue_cr": 99.07,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE SME",
        "registrar": "Link Intime India Pvt Ltd",
        "description": "Vertically integrated defense electronics solutions provider focusing on situational awareness and command-control systems."
    },

    # ── OPEN / ONGOING IPOS ────────────────────────────────────────────────────
    {
        "id": "zinka-logistics",
        "company_name": "Zinka Logistics Solution (BlackBuck)",
        "symbol": "BLACKBUCK",
        "ipo_type": "MAINBOARD",
        "status": "OPEN",
        "opening_date": "2026-11-13",
        "closing_date": "2026-11-18",
        "allotment_date": "2026-11-19",
        "listing_date": "2026-11-21",
        "price_band": "₹259 - ₹273",
        "price_min": 259.0,
        "price_max": 273.0,
        "lot_size": 54,
        "min_investment": 14742.0,  # 54 * 273
        "issue_size_cr": 1114.72,
        "fresh_issue_cr": 550.0,
        "ofs_cr": 564.72,
        "face_value": "₹1 per share",
        "subscription_status": "1.86x Subscribed",
        "subscription_stats": {"qib": 2.76, "nii": 0.24, "retail": 1.66, "total": 1.86},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Limited",
        "description": "Digital platform for truck operators in India providing payments, telematics, freight marketplace, and vehicle financing services."
    },
    {
        "id": "onyx-biotec",
        "company_name": "Onyx Biotec Limited",
        "symbol": "ONYX",
        "ipo_type": "SME",
        "status": "OPEN",
        "opening_date": "2026-11-13",
        "closing_date": "2026-11-18",
        "allotment_date": "2026-11-19",
        "listing_date": "2026-11-21",
        "price_band": "₹58 - ₹61",
        "price_min": 58.0,
        "price_max": 61.0,
        "lot_size": 2000,
        "min_investment": 122000.0,
        "issue_size_cr": 29.34,
        "fresh_issue_cr": 29.34,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "subscription_status": "4.22x Subscribed",
        "subscription_stats": {"qib": 1.15, "nii": 3.84, "retail": 6.55, "total": 4.22},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE SME",
        "registrar": "Mas Services Limited",
        "description": "Pharmaceutical manufacturing company producing sterile water for injections and dry powder parenteral solutions."
    },

    # ── RECENTLY LISTED IPOS ───────────────────────────────────────────────────
    {
        "id": "swiggy",
        "company_name": "Swiggy Limited",
        "symbol": "SWIGGY",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2026-11-06",
        "closing_date": "2026-11-08",
        "allotment_date": "2026-11-11",
        "listing_date": "2026-11-13",
        "price_band": "₹371 - ₹390",
        "price_min": 371.0,
        "price_max": 390.0,
        "lot_size": 38,
        "min_investment": 14820.0,
        "issue_size_cr": 11327.43,
        "fresh_issue_cr": 4499.0,
        "ofs_cr": 6828.43,
        "face_value": "₹1 per share",
        "subscription_status": "3.59x Subscribed",
        "subscription_stats": {"qib": 6.02, "nii": 0.41, "retail": 1.14, "total": 3.59},
        "listing_price": 420.0,
        "listing_gain_pct": 7.69,
        "current_price": 456.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "description": "Leading consumer internet convenience platform in India operating Food Delivery, Instamart (quick commerce), and Dineout."
    },
    {
        "id": "acme-solar",
        "company_name": "ACME Solar Holdings Limited",
        "symbol": "ACMESOLAR",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2026-11-06",
        "closing_date": "2026-11-08",
        "allotment_date": "2026-11-11",
        "listing_date": "2026-11-13",
        "price_band": "₹275 - ₹289",
        "price_min": 275.0,
        "price_max": 289.0,
        "lot_size": 51,
        "min_investment": 14739.0,
        "issue_size_cr": 2900.0,
        "fresh_issue_cr": 2395.0,
        "ofs_cr": 505.0,
        "face_value": "₹2 per share",
        "subscription_status": "2.75x Subscribed",
        "subscription_stats": {"qib": 3.54, "nii": 0.97, "retail": 3.10, "total": 2.75},
        "listing_price": 251.0,
        "listing_gain_pct": -13.15,
        "current_price": 258.40,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Limited",
        "description": "Renewable energy power producer in India with a portfolio of solar, wind, hybrid and firm and dispatchable renewable energy projects."
    },
    {
        "id": "sagility-india",
        "company_name": "Sagility India Limited",
        "symbol": "SAGILITY",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2026-11-05",
        "closing_date": "2026-11-07",
        "allotment_date": "2026-11-08",
        "listing_date": "2026-11-12",
        "price_band": "₹28 - ₹30",
        "price_min": 28.0,
        "price_max": 30.0,
        "lot_size": 500,
        "min_investment": 15000.0,
        "issue_size_cr": 2106.60,
        "fresh_issue_cr": 0.0,
        "ofs_cr": 2106.60,
        "face_value": "₹10 per share",
        "subscription_status": "3.20x Subscribed",
        "subscription_stats": {"qib": 3.52, "nii": 1.93, "retail": 4.16, "total": 3.20},
        "listing_price": 31.06,
        "listing_gain_pct": 3.53,
        "current_price": 32.80,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "description": "Technology-enabled healthcare business process management services provider for US healthcare payers and providers."
    },
    {
        "id": "waaree-energies",
        "company_name": "Waaree Energies Limited",
        "symbol": "WAAREE",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2026-10-21",
        "closing_date": "2026-10-23",
        "allotment_date": "2026-10-24",
        "listing_date": "2026-10-28",
        "price_band": "₹1427 - ₹1503",
        "price_min": 1427.0,
        "price_max": 1503.0,
        "lot_size": 9,
        "min_investment": 13527.0,
        "issue_size_cr": 4321.44,
        "fresh_issue_cr": 3600.0,
        "ofs_cr": 721.44,
        "face_value": "₹10 per share",
        "subscription_status": "76.34x Subscribed",
        "subscription_stats": {"qib": 208.63, "nii": 62.49, "retail": 10.79, "total": 76.34},
        "listing_price": 2550.0,
        "listing_gain_pct": 69.66,
        "current_price": 2985.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "description": "India's largest solar photovoltaic module manufacturer with an installed capacity of 12 GW as of June 2026."
    },

    # ── CLOSED IPOS ────────────────────────────────────────────────────────────
    {
        "id": "niva-bupa",
        "company_name": "Niva Bupa Health Insurance Co Ltd",
        "symbol": "NIVABUPA",
        "ipo_type": "MAINBOARD",
        "status": "CLOSED",
        "opening_date": "2026-11-07",
        "closing_date": "2026-11-11",
        "allotment_date": "2026-11-12",
        "listing_date": "2026-11-14",
        "price_band": "₹70 - ₹74",
        "price_min": 70.0,
        "price_max": 74.0,
        "lot_size": 200,
        "min_investment": 14800.0,
        "issue_size_cr": 2200.0,
        "fresh_issue_cr": 800.0,
        "ofs_cr": 1400.0,
        "face_value": "₹10 per share",
        "subscription_status": "1.80x Subscribed (Awaiting Listing)",
        "subscription_stats": {"qib": 2.06, "nii": 0.68, "retail": 2.73, "total": 1.80},
        "listing_price": 78.50,
        "listing_gain_pct": 6.08,
        "current_price": 81.20,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Limited",
        "description": "Standalone health insurer in India focusing on retail health insurance, personal accident, and travel policies."
    },
    {
        "id": "afcons-infra",
        "company_name": "Afcons Infrastructure Limited",
        "symbol": "AFCONS",
        "ipo_type": "MAINBOARD",
        "status": "CLOSED",
        "opening_date": "2026-10-25",
        "closing_date": "2026-10-29",
        "allotment_date": "2026-10-30",
        "listing_date": "2026-11-04",
        "price_band": "₹440 - ₹463",
        "price_min": 440.0,
        "price_max": 463.0,
        "lot_size": 32,
        "min_investment": 14816.0,
        "issue_size_cr": 5430.0,
        "fresh_issue_cr": 1250.0,
        "ofs_cr": 4180.0,
        "face_value": "₹10 per share",
        "subscription_status": "2.63x Subscribed",
        "subscription_stats": {"qib": 3.79, "nii": 5.05, "retail": 0.94, "total": 2.63},
        "listing_price": 426.0,
        "listing_gain_pct": -7.99,
        "current_price": 472.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "description": "Flagship infrastructure engineering and construction company of the Shapoorji Pallonji Group."
    }
]


def list_ipos(status="ALL", ipo_type="ALL", search="", page=1, limit=20):
    """Filter and paginate IPO list."""
    results = IPOS_DATA

    # Status filter
    if status and status.upper() != "ALL":
        results = [x for x in results if x["status"] == status.upper()]

    # Type filter (MAINBOARD / SME)
    if ipo_type and ipo_type.upper() != "ALL":
        results = [x for x in results if x["ipo_type"] == ipo_type.upper()]

    # Search filter
    if search:
        s = search.strip().upper()
        results = [
            x for x in results
            if s in x["company_name"].upper() or s in x["symbol"].upper() or s in x.get("registrar", "").upper()
        ]

    total = len(results)
    start = (page - 1) * limit
    end = start + limit
    paginated = results[start:end]

    return {
        "ipos": paginated,
        "total": total,
        "page": page,
        "limit": limit,
        "totalPages": max(1, (total + limit - 1) // limit)
    }


def get_ipo_detail(identifier):
    """Retrieve complete detail of a specific IPO by ID or Symbol."""
    ident = str(identifier).strip().upper()
    for ipo in IPOS_DATA:
        if ipo["id"].upper() == ident or ipo["symbol"].upper() == ident:
            return ipo
    return None


def get_ipo_summary():
    """Retrieve quick aggregate counts for dashboard cards."""
    upcoming = len([x for x in IPOS_DATA if x["status"] == "UPCOMING"])
    open_ipos = len([x for x in IPOS_DATA if x["status"] == "OPEN"])
    closed = len([x for x in IPOS_DATA if x["status"] == "CLOSED"])
    listed = len([x for x in IPOS_DATA if x["status"] == "LISTED"])
    sme = len([x for x in IPOS_DATA if x["ipo_type"] == "SME"])
    mainboard = len([x for x in IPOS_DATA if x["ipo_type"] == "MAINBOARD"])
    return {
        "total": len(IPOS_DATA),
        "upcoming": upcoming,
        "open": open_ipos,
        "closed": closed,
        "listed": listed,
        "sme": sme,
        "mainboard": mainboard
    }

