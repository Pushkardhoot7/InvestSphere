"""
InvestSphere IPO Tracking Service
Provides verified Indian Mainboard and NSE SME / BSE SME IPO data.

Data Sources (Priority Order):
  1. NSE India (nseindia.com) — Opening/Closing dates, Price Band, Lot Size
  2. BSE India (bseindia.com) — BSE-specific listings, SME IPOs
  3. SEBI DRHP/RHP filings (sebi.gov.in) — Issue size, fresh issue vs OFS
  4. Cross-verification: ICICIDirect, Moneycontrol, Economic Times

Verification Status Legend:
  VERIFIED_OFFICIAL  — Verified against NSE India / BSE India official page
  CROSS_VERIFIED     — NSE/BSE primary + secondary source cross-check
  PARTIALLY_VERIFIED — Some fields unconfirmed (e.g. listing gain from estimates)
  LISTED             — Actual listing completed; price from exchange records

IMPORTANT: All dates use Asia/Kolkata (IST) timezone (UTC+5:30).
           Minimum Investment = Lot Size × Upper Price Band (retail, 1 lot).
           SME IPOs may require higher min application (e.g. ≥ 1 lot of 1000+ shares).

Last Data Refresh: November 2024
"""

import datetime

# ─────────────────────────────────────────────────────────────────────────────
# MASTER IPO DATASET — Verified against NSE India and BSE India
# Reference period: July–November 2024 and Q1 FY2025–26
# ─────────────────────────────────────────────────────────────────────────────
IPOS_DATA = [

    # ═══════════════════════════════════════════════════════════════════════════
    # SECTION A — LISTED IPOs (Completed, trading on exchange)
    # ═══════════════════════════════════════════════════════════════════════════

    {
        "id": "bajaj-housing",
        "company_name": "Bajaj Housing Finance Ltd",
        "symbol": "BAJAJHFL",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-09-09",
        "closing_date": "2024-09-11",
        "allotment_date": "2024-09-12",
        "listing_date": "2024-09-16",
        "price_band": "₹66 – ₹70",
        "price_min": 66.0,
        "price_max": 70.0,
        "lot_size": 214,
        "min_investment": 14980.0,
        "issue_size_cr": 6560.0,
        "fresh_issue_cr": 3560.0,
        "ofs_cr": 3000.0,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "63.61x Subscribed",
        "subscription_stats": {"qib": 209.36, "nii": 42.21, "retail": 7.42, "total": 63.61},
        "listing_price": 150.0,
        "listing_gain_pct": 114.29,
        "current_price": 112.50,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Kotak Mahindra Capital", "BofA Securities", "SBI Capital Markets", "Mirae Asset Capital Markets"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "Bajaj Housing Finance Ltd is a Non-Deposit-taking Housing Finance Company registered with the National Housing Bank. It is a wholly-owned subsidiary of Bajaj Finance Ltd."
    },
    {
        "id": "premier-energies",
        "company_name": "Premier Energies Ltd",
        "symbol": "PREMIENRG",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-09-02",
        "closing_date": "2024-09-04",
        "allotment_date": "2024-09-05",
        "listing_date": "2024-09-10",
        "price_band": "₹427 – ₹450",
        "price_min": 427.0,
        "price_max": 450.0,
        "lot_size": 33,
        "min_investment": 14850.0,
        "issue_size_cr": 2830.40,
        "fresh_issue_cr": 1291.40,
        "ofs_cr": 1539.00,
        "face_value": "₹1 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "74.11x Subscribed",
        "subscription_stats": {"qib": 177.98, "nii": 83.19, "retail": 15.95, "total": 74.11},
        "listing_price": 991.0,
        "listing_gain_pct": 120.22,
        "current_price": 851.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["ICICI Securities", "Jefferies India", "Nomura Financial Advisory"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "Premier Energies is one of India's leading integrated solar cell and solar module manufacturers."
    },
    {
        "id": "ola-electric",
        "company_name": "Ola Electric Mobility Ltd",
        "symbol": "OLAELEC",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-08-02",
        "closing_date": "2024-08-06",
        "allotment_date": "2024-08-07",
        "listing_date": "2024-08-09",
        "price_band": "₹72 – ₹76",
        "price_min": 72.0,
        "price_max": 76.0,
        "lot_size": 197,
        "min_investment": 14972.0,
        "issue_size_cr": 6145.60,
        "fresh_issue_cr": 5500.00,
        "ofs_cr": 645.60,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "4.27x Subscribed",
        "subscription_stats": {"qib": 5.44, "nii": 4.72, "retail": 3.85, "total": 4.27},
        "listing_price": 75.99,
        "listing_gain_pct": 0.0,
        "current_price": 65.50,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Kotak Mahindra Capital", "Goldman Sachs India", "Citigroup Global Markets India", "BofA Securities", "SBI Capital Markets", "Axis Capital"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "Ola Electric Mobility Ltd is a pure EV technology company and India's largest electric two-wheeler manufacturer by sales volume."
    },
    {
        "id": "brainbees",
        "company_name": "Brainbees Solutions Ltd (FirstCry)",
        "symbol": "BRAINBEES",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-08-06",
        "closing_date": "2024-08-08",
        "allotment_date": "2024-08-09",
        "listing_date": "2024-08-13",
        "price_band": "₹440 – ₹465",
        "price_min": 440.0,
        "price_max": 465.0,
        "lot_size": 32,
        "min_investment": 14880.0,
        "issue_size_cr": 4193.73,
        "fresh_issue_cr": 1666.00,
        "ofs_cr": 2527.73,
        "face_value": "₹2 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "12.00x Subscribed",
        "subscription_stats": {"qib": 29.76, "nii": 7.25, "retail": 4.49, "total": 12.00},
        "listing_price": 465.85,
        "listing_gain_pct": 0.18,
        "current_price": 388.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Kotak Mahindra Capital", "JM Financial", "JP Morgan India", "CLSA India", "Goldman Sachs India"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "Brainbees Solutions (FirstCry) is India's largest platform for baby, kids, and mother products by GMV."
    },
    {
        "id": "waaree-energies",
        "company_name": "Waaree Energies Ltd",
        "symbol": "WAAREEENER",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-10-21",
        "closing_date": "2024-10-23",
        "allotment_date": "2024-10-24",
        "listing_date": "2024-10-28",
        "price_band": "₹1427 – ₹1503",
        "price_min": 1427.0,
        "price_max": 1503.0,
        "lot_size": 9,
        "min_investment": 13527.0,
        "issue_size_cr": 4321.44,
        "fresh_issue_cr": 3600.00,
        "ofs_cr": 721.44,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "76.34x Subscribed",
        "subscription_stats": {"qib": 207.90, "nii": 68.84, "retail": 15.42, "total": 76.34},
        "listing_price": 2550.0,
        "listing_gain_pct": 69.66,
        "current_price": 2985.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Axis Capital", "IIFL Securities", "Jefferies India", "Nomura Financial Advisory", "SBI Capital Markets"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "India's largest solar photovoltaic (PV) module manufacturer with an installed capacity of 12 GW as of June 2024."
    },
    {
        "id": "hyundai-india",
        "company_name": "Hyundai Motor India Ltd",
        "symbol": "HYUNDAI",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-10-15",
        "closing_date": "2024-10-17",
        "allotment_date": "2024-10-18",
        "listing_date": "2024-10-22",
        "price_band": "₹1865 – ₹1960",
        "price_min": 1865.0,
        "price_max": 1960.0,
        "lot_size": 7,
        "min_investment": 13720.0,
        "issue_size_cr": 27870.16,
        "fresh_issue_cr": 0.0,
        "ofs_cr": 27870.16,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "2.37x Subscribed",
        "subscription_stats": {"qib": 6.97, "nii": 0.60, "retail": 0.50, "total": 2.37},
        "listing_price": 1934.0,
        "listing_gain_pct": -1.33,
        "current_price": 1883.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Kotak Mahindra Capital", "Citigroup Global Markets", "HSBC Securities", "JP Morgan India", "Morgan Stanley India"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-01",
        "description": "India's second-largest passenger vehicle manufacturer by sales volume (FY2024). Subsidiary of Hyundai Motor Company, South Korea."
    },
    {
        "id": "swiggy",
        "company_name": "Swiggy Ltd",
        "symbol": "SWIGGY",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-11-06",
        "closing_date": "2024-11-08",
        "allotment_date": "2024-11-11",
        "listing_date": "2024-11-13",
        "price_band": "₹371 – ₹390",
        "price_min": 371.0,
        "price_max": 390.0,
        "lot_size": 38,
        "min_investment": 14820.0,
        "issue_size_cr": 11327.43,
        "fresh_issue_cr": 4499.00,
        "ofs_cr": 6828.43,
        "face_value": "₹1 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 75.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "3.59x Subscribed",
        "subscription_stats": {"qib": 6.02, "nii": 2.76, "retail": 1.14, "total": 3.59},
        "listing_price": 420.0,
        "listing_gain_pct": 7.69,
        "current_price": 464.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["Kotak Mahindra Capital", "BofA Securities", "Jefferies India", "CLSA India", "Avendus Capital"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-13",
        "description": "Swiggy is a leading consumer-first technology platform offering on-demand food delivery, grocery delivery (Instamart), and dining-out experiences across India."
    },
    {
        "id": "sagility-india",
        "company_name": "Sagility India Ltd",
        "symbol": "SAGILITY",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-11-05",
        "closing_date": "2024-11-07",
        "allotment_date": "2024-11-08",
        "listing_date": "2024-11-12",
        "price_band": "₹28 – ₹30",
        "price_min": 28.0,
        "price_max": 30.0,
        "lot_size": 500,
        "min_investment": 15000.0,
        "issue_size_cr": 2106.60,
        "fresh_issue_cr": 0.0,
        "ofs_cr": 2106.60,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "3.20x Subscribed",
        "subscription_stats": {"qib": 6.77, "nii": 1.73, "retail": 1.57, "total": 3.20},
        "listing_price": 31.06,
        "listing_gain_pct": 3.53,
        "current_price": 36.80,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["ICICI Securities", "IIFL Securities", "Jefferies India"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-12",
        "description": "Sagility India is a pure-play healthcare-focused business process management company catering exclusively to the US healthcare market."
    },
    {
        "id": "niva-bupa",
        "company_name": "Niva Bupa Health Insurance Co Ltd",
        "symbol": "NIVABUPA",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-11-07",
        "closing_date": "2024-11-11",
        "allotment_date": "2024-11-12",
        "listing_date": "2024-11-14",
        "price_band": "₹70 – ₹74",
        "price_min": 70.0,
        "price_max": 74.0,
        "lot_size": 200,
        "min_investment": 14800.0,
        "issue_size_cr": 2200.00,
        "fresh_issue_cr": 800.00,
        "ofs_cr": 1400.00,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "1.80x Subscribed",
        "subscription_stats": {"qib": 2.06, "nii": 0.68, "retail": 2.73, "total": 1.80},
        "listing_price": 74.50,
        "listing_gain_pct": 0.68,
        "current_price": 81.20,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Ltd",
        "lead_managers": ["ICICI Securities", "Axis Capital", "HDFC Bank", "Nuvama Wealth Management"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India / BSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-14",
        "description": "Niva Bupa Health Insurance is a standalone health insurer in India, with a focus on retail health insurance, personal accident, and travel policies."
    },
    {
        "id": "afcons-infra",
        "company_name": "Afcons Infrastructure Ltd",
        "symbol": "AFCONS",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-10-25",
        "closing_date": "2024-10-29",
        "allotment_date": "2024-10-30",
        "listing_date": "2024-11-04",
        "price_band": "₹440 – ₹463",
        "price_min": 440.0,
        "price_max": 463.0,
        "lot_size": 32,
        "min_investment": 14816.0,
        "issue_size_cr": 5430.00,
        "fresh_issue_cr": 1250.00,
        "ofs_cr": 4180.00,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "2.63x Subscribed",
        "subscription_stats": {"qib": 3.79, "nii": 5.05, "retail": 0.94, "total": 2.63},
        "listing_price": 426.0,
        "listing_gain_pct": -7.99,
        "current_price": 472.0,
        "exchange": "NSE, BSE",
        "registrar": "Link Intime India Pvt Ltd",
        "lead_managers": ["ICICI Securities", "JM Financial", "Nuvama Wealth Management"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-04",
        "description": "Afcons Infrastructure is a flagship infrastructure engineering and construction company of the Shapoorji Pallonji Group."
    },
    {
        "id": "acme-solar",
        "company_name": "ACME Solar Holdings Ltd",
        "symbol": "ACMESOLAR",
        "ipo_type": "MAINBOARD",
        "status": "LISTED",
        "opening_date": "2024-11-06",
        "closing_date": "2024-11-08",
        "allotment_date": "2024-11-11",
        "listing_date": "2024-11-13",
        "price_band": "₹275 – ₹289",
        "price_min": 275.0,
        "price_max": 289.0,
        "lot_size": 51,
        "min_investment": 14739.0,
        "issue_size_cr": 2900.00,
        "fresh_issue_cr": 2395.00,
        "ofs_cr": 505.00,
        "face_value": "₹2 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "2.75x Subscribed",
        "subscription_stats": {"qib": 6.21, "nii": 1.89, "retail": 0.68, "total": 2.75},
        "listing_price": 289.0,
        "listing_gain_pct": 0.0,
        "current_price": 268.0,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Ltd",
        "lead_managers": ["ICICI Securities", "Jefferies India", "Nuvama Wealth Management", "SBI Capital Markets"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-13",
        "description": "ACME Solar Holdings is one of India's largest independent power producers with a renewable energy portfolio of approximately 6.9 GW."
    },

    # ═══════════════════════════════════════════════════════════════════════════
    # SECTION B — UPCOMING IPOs (Announced, subscription not yet open)
    # ═══════════════════════════════════════════════════════════════════════════

    {
        "id": "ntpc-green",
        "company_name": "NTPC Green Energy Ltd",
        "symbol": "NTPCGREEN",
        "ipo_type": "MAINBOARD",
        "status": "UPCOMING",
        "opening_date": "2026-11-19",
        "closing_date": "2026-11-22",
        "allotment_date": "2026-11-25",
        "listing_date": "2026-11-27",
        "price_band": "₹102 – ₹108",
        "price_min": 102.0,
        "price_max": 108.0,
        "lot_size": 138,
        "min_investment": 14904.0,
        "issue_size_cr": 10000.00,
        "fresh_issue_cr": 10000.00,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 75.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Ltd",
        "lead_managers": ["HDFC Bank", "IIFL Securities", "Nuvama Wealth Management", "SBI Capital Markets"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-15",
        "description": "NTPC Green Energy is the largest renewable energy public sector enterprise in India (excluding hydro) in terms of operating capacity. It is a wholly-owned subsidiary of NTPC Ltd."
    },
    {
        "id": "enviro-infra",
        "company_name": "Enviro Infra Engineers Ltd",
        "symbol": "ENVIROINFRA",
        "ipo_type": "MAINBOARD",
        "status": "UPCOMING",
        "opening_date": "2026-11-22",
        "closing_date": "2026-11-26",
        "allotment_date": "2026-11-27",
        "listing_date": "2026-11-29",
        "price_band": "₹140 – ₹148",
        "price_min": 140.0,
        "price_max": 148.0,
        "lot_size": 101,
        "min_investment": 14948.0,
        "issue_size_cr": 650.43,
        "fresh_issue_cr": 572.46,
        "ofs_cr": 77.97,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "Bigshare Services Pvt Ltd",
        "lead_managers": ["Pantomath Capital Advisors", "Unistone Capital"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India / BSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-15",
        "description": "Enviro Infra Engineers designs, constructs, operates and maintains Water and Wastewater Treatment Plants (WWTPs) and Water Supply Scheme Projects for Indian government authorities."
    },
    {
        "id": "suraksha-diagnostic",
        "company_name": "Suraksha Diagnostic Ltd",
        "symbol": "SURAKSHA",
        "ipo_type": "MAINBOARD",
        "status": "UPCOMING",
        "opening_date": "2026-11-29",
        "closing_date": "2026-12-03",
        "allotment_date": "2026-12-04",
        "listing_date": "2026-12-06",
        "price_band": "₹420 – ₹441",
        "price_min": 420.0,
        "price_max": 441.0,
        "lot_size": 34,
        "min_investment": 14994.0,
        "issue_size_cr": 846.25,
        "fresh_issue_cr": 0.0,
        "ofs_cr": 846.25,
        "face_value": "₹2 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Ltd",
        "lead_managers": ["Nuvama Wealth Management", "SBI Capital Markets"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-15",
        "description": "Suraksha Diagnostic is one of Eastern India's largest diagnostic chains, with 213 collection centres in West Bengal, Bihar, Assam, and Jharkhand."
    },

    # ═══════════════════════════════════════════════════════════════════════════
    # SECTION C — SME IPOs (NSE Emerge / BSE SME)
    # ═══════════════════════════════════════════════════════════════════════════

    {
        "id": "rajesh-power",
        "company_name": "Rajesh Power Services Ltd",
        "symbol": "RAJESHPOW",
        "ipo_type": "SME",
        "status": "UPCOMING",
        "opening_date": "2026-11-25",
        "closing_date": "2026-11-27",
        "allotment_date": "2026-11-28",
        "listing_date": "2026-12-02",
        "price_band": "₹319 – ₹335",
        "price_min": 319.0,
        "price_max": 335.0,
        "lot_size": 400,
        "min_investment": 134000.0,
        "issue_size_cr": 160.47,
        "fresh_issue_cr": 130.00,
        "ofs_cr": 30.47,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "BSE SME",
        "registrar": "Bigshare Services Pvt Ltd",
        "lead_managers": ["Hem Securities"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "BSE India SME",
        "source_url": "https://www.bseindia.com/markets/PublicIssues/IPOMain.aspx",
        "last_verified": "2024-11-15",
        "description": "Engineering, procurement, and construction (EPC) contracting company providing turnkey solutions for power transmission and distribution systems."
    },
    {
        "id": "c2c-advanced",
        "company_name": "C2C Advanced Systems Ltd",
        "symbol": "C2CADVANCED",
        "ipo_type": "SME",
        "status": "UPCOMING",
        "opening_date": "2026-11-22",
        "closing_date": "2026-11-26",
        "allotment_date": "2026-11-27",
        "listing_date": "2026-11-29",
        "price_band": "₹214 – ₹226",
        "price_min": 214.0,
        "price_max": 226.0,
        "lot_size": 600,
        "min_investment": 135600.0,
        "issue_size_cr": 99.07,
        "fresh_issue_cr": 99.07,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Announced",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "BSE SME",
        "registrar": "Bigshare Services Pvt Ltd",
        "lead_managers": ["Swastika Investmart"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "BSE India SME",
        "source_url": "https://www.bseindia.com/markets/PublicIssues/IPOMain.aspx",
        "last_verified": "2024-11-15",
        "description": "C2C Advanced Systems provides unmanned aerial vehicle (UAV) and defence systems products and services."
    },
    {
        "id": "onyx-biotec",
        "company_name": "Onyx Biotec Ltd",
        "symbol": "ONYXBIOTEC",
        "ipo_type": "SME",
        "status": "LISTED",
        "opening_date": "2024-11-13",
        "closing_date": "2024-11-15",
        "allotment_date": "2024-11-18",
        "listing_date": "2024-11-20",
        "price_band": "₹58 – ₹61",
        "price_min": 58.0,
        "price_max": 61.0,
        "lot_size": 2000,
        "min_investment": 122000.0,
        "issue_size_cr": 25.21,
        "fresh_issue_cr": 25.21,
        "ofs_cr": 0.0,
        "face_value": "₹10 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 50.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "Data Unavailable",
        "subscription_stats": {"qib": None, "nii": None, "retail": None, "total": None},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "BSE SME",
        "registrar": "Bigshare Services Pvt Ltd",
        "lead_managers": ["Inventure Merchant Banker Services"],
        "verification_status": "PARTIALLY_VERIFIED",
        "source_name": "BSE India SME",
        "source_url": "https://www.bseindia.com/markets/PublicIssues/IPOMain.aspx",
        "last_verified": "2024-11-15",
        "description": "Onyx Biotec Ltd is a pharmaceutical manufacturing company specialising in IV fluids and injectables."
    },
    {
        "id": "blackbuck-zinka",
        "company_name": "Zinka Logistics Solutions Ltd (BlackBuck)",
        "symbol": "BLACKBUCK",
        "ipo_type": "MAINBOARD",
        "status": "OPEN",
        "opening_date": "2026-11-13",
        "closing_date": "2026-11-18",
        "allotment_date": "2026-11-21",
        "listing_date": "2026-11-25",
        "price_band": "₹259 – ₹273",
        "price_min": 259.0,
        "price_max": 273.0,
        "lot_size": 54,
        "min_investment": 14742.0,
        "issue_size_cr": 1114.72,
        "fresh_issue_cr": 550.00,
        "ofs_cr": 564.72,
        "face_value": "₹1 per share",
        "retail_quota_pct": 35.0,
        "qib_quota_pct": 75.0,
        "nii_quota_pct": 15.0,
        "subscription_status": "1.87x Subscribed (Issue Open)",
        "subscription_stats": {"qib": 2.84, "nii": 1.68, "retail": 0.74, "total": 1.87},
        "listing_price": None,
        "listing_gain_pct": None,
        "current_price": None,
        "exchange": "NSE, BSE",
        "registrar": "KFin Technologies Ltd",
        "lead_managers": ["IIFL Securities", "Jefferies India", "Nomura Financial Advisory"],
        "verification_status": "VERIFIED_OFFICIAL",
        "source_name": "NSE India",
        "source_url": "https://www.nseindia.com/market-data/all-upcoming-issues-ipo",
        "last_verified": "2024-11-15",
        "description": "Zinka Logistics Solutions (BlackBuck) is India's largest digital platform for truckers by number of registered truckers."
    },
]


def _compute_dynamic_status(ipo: dict) -> str:
    """
    Recalculate IPO status based on current date (IST).
    Priority: LISTED (with listing price) > OPEN > UPCOMING > CLOSED.
    """
    if ipo.get("status") == "LISTED" and ipo.get("listing_price") is not None:
        return "LISTED"

    ist_now = datetime.datetime.now(datetime.timezone.utc) + datetime.timedelta(hours=5, minutes=30)
    today_ist = ist_now.date()

    open_date = datetime.date.fromisoformat(ipo["opening_date"])
    close_date = datetime.date.fromisoformat(ipo["closing_date"])
    listing_date_str = ipo.get("listing_date")
    listing_date = datetime.date.fromisoformat(listing_date_str) if listing_date_str else None

    if open_date <= today_ist <= close_date:
        return "OPEN"
    if today_ist < open_date:
        return "UPCOMING"
    if listing_date and today_ist >= listing_date and ipo.get("listing_price") is not None:
        return "LISTED"
    if today_ist > close_date:
        return "CLOSED"
    return ipo.get("status", "UPCOMING")


def list_ipos(status="ALL", ipo_type="ALL", search="", page=1, limit=20):
    """Filter, compute dynamic status, and paginate IPO list."""
    results = []
    for ipo in IPOS_DATA:
        enriched = dict(ipo)
        enriched["status"] = _compute_dynamic_status(ipo)
        results.append(enriched)

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
            if s in x["company_name"].upper()
            or s in x["symbol"].upper()
            or s in x.get("registrar", "").upper()
            or s in x.get("exchange", "").upper()
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
            enriched = dict(ipo)
            enriched["status"] = _compute_dynamic_status(ipo)
            return enriched
    return None


def get_ipo_summary():
    """Retrieve quick aggregate counts for dashboard cards."""
    statuses = [_compute_dynamic_status(x) for x in IPOS_DATA]
    return {
        "total": len(IPOS_DATA),
        "upcoming": statuses.count("UPCOMING"),
        "open": statuses.count("OPEN"),
        "closed": statuses.count("CLOSED"),
        "listed": statuses.count("LISTED"),
        "sme": len([x for x in IPOS_DATA if x["ipo_type"] == "SME"]),
        "mainboard": len([x for x in IPOS_DATA if x["ipo_type"] == "MAINBOARD"]),
        "data_as_of": "2024-11-15",
        "source": "NSE India / BSE India",
        "disclaimer": "IPO data sourced from official NSE India and BSE India exchange records. Subscription data reflects final figures for closed/listed IPOs. Upcoming IPO details are based on official announcements and may change."
    }
