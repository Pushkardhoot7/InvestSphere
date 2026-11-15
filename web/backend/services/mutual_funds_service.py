"""
InvestSphere Mutual Funds & SIP Advisory Service
Comprehensive catalog of Indian mutual fund schemes across Equity, Hybrid, Debt, and Index/Passive categories.
Includes high-precision mathematical SIP engine with annual step-up, side-by-side fund comparison,
and transparent goal-based recommendation wizard.
"""

import math
import datetime

# ─────────────────────────────────────────────────────────────────────────────
# Curated Indian Mutual Fund Scheme Catalog
# Direct Plans / Growth Options with realistic benchmark metrics
# ─────────────────────────────────────────────────────────────────────────────

SCHEMES_CATALOG = [
    # ── EQUITY: FLEXI CAP & MULTI CAP ──────────────────────────────────────────
    {
        "id": "ppfas-flexi-cap",
        "name": "Parag Parikh Flexi Cap Fund - Direct Plan - Growth",
        "short_name": "Parag Parikh Flexi Cap",
        "amc": "PPFAS Mutual Fund",
        "category": "Flexi Cap",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 79.42,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.64,
        "min_sip": 1000,
        "returns_1y": 28.4,
        "returns_3y": 21.6,
        "returns_5y": 22.8,
        "returns_10y": 19.5,
        "benchmark": "NIFTY 500 TRI",
        "risk": "Very High",
        "fund_manager": "Rajeev Thakkar & Raunak Onkar",
        "aum_cr": 78500.0,
        "rating": 5,
        "top_holdings": ["HDFC Bank", "ITC", "Bajaj Holdings", "Alphabet Inc", "Microsoft"],
        "description": "Flagship value-oriented flexi cap investing dynamically across large, mid, small caps and selective global tech leaders."
    },
    {
        "id": "hdfc-flexi-cap",
        "name": "HDFC Flexi Cap Fund - Direct Plan - Growth",
        "short_name": "HDFC Flexi Cap",
        "amc": "HDFC Mutual Fund",
        "category": "Flexi Cap",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 1845.20,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.78,
        "min_sip": 500,
        "returns_1y": 32.1,
        "returns_3y": 24.5,
        "returns_5y": 21.9,
        "returns_10y": 16.8,
        "benchmark": "NIFTY 500 TRI",
        "risk": "Very High",
        "fund_manager": "Roshi Jain",
        "aum_cr": 64200.0,
        "rating": 5,
        "top_holdings": ["ICICI Bank", "HDFC Bank", "Infosys", "Bharti Airtel", "SBI"],
        "description": "Consistent market-leading fund focusing on compounding businesses with high operational moats."
    },

    # ── EQUITY: LARGE CAP & INDEX ──────────────────────────────────────────────
    {
        "id": "uti-nifty-50-index",
        "name": "UTI Nifty 50 Index Fund - Direct Plan - Growth",
        "short_name": "UTI Nifty 50 Index",
        "amc": "UTI Mutual Fund",
        "category": "Index / Passive",
        "asset_class": "INDEX",
        "plan": "Direct",
        "option": "Growth",
        "nav": 178.60,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.18,
        "min_sip": 500,
        "returns_1y": 24.2,
        "returns_3y": 16.8,
        "returns_5y": 17.5,
        "returns_10y": 14.8,
        "benchmark": "NIFTY 50 TRI",
        "risk": "Very High",
        "fund_manager": "Sharwan Kumar Goyal",
        "aum_cr": 21500.0,
        "rating": 4,
        "top_holdings": ["HDFC Bank", "Reliance", "ICICI Bank", "Infosys", "TCS"],
        "description": "Low-cost passive tracking of India's top 50 bluechip companies with ultra-low tracking error."
    },
    {
        "id": "mirae-large-cap",
        "name": "Mirae Asset Large Cap Fund - Direct Plan - Growth",
        "short_name": "Mirae Asset Large Cap",
        "amc": "Mirae Asset Mutual Fund",
        "category": "Large Cap",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 118.40,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.52,
        "min_sip": 1000,
        "returns_1y": 23.8,
        "returns_3y": 16.2,
        "returns_5y": 16.9,
        "returns_10y": 15.6,
        "benchmark": "NIFTY 100 TRI",
        "risk": "Very High",
        "fund_manager": "Gaurav Misra",
        "aum_cr": 38900.0,
        "rating": 4,
        "top_holdings": ["HDFC Bank", "ICICI Bank", "Reliance", "L&T", "TCS"],
        "description": "High-conviction disciplined investment in India's top 100 market capitalization leaders."
    },

    # ── EQUITY: MID CAP & SMALL CAP ────────────────────────────────────────────
    {
        "id": "motilal-midcap",
        "name": "Motilal Oswal Midcap Fund - Direct Plan - Growth",
        "short_name": "Motilal Oswal Midcap",
        "amc": "Motilal Oswal Mutual Fund",
        "category": "Mid Cap",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 108.50,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.68,
        "min_sip": 500,
        "returns_1y": 48.6,
        "returns_3y": 34.2,
        "returns_5y": 28.5,
        "returns_10y": 20.8,
        "benchmark": "NIFTY Midcap 150 TRI",
        "risk": "Very High",
        "fund_manager": "Niket Shah",
        "aum_cr": 16800.0,
        "rating": 5,
        "top_holdings": ["Persistent Systems", "Trent", "Coforge", "Kalyan Jewellers", "Dixon Tech"],
        "description": "Aggressive midcap growth fund targeting fast-scaling category leaders."
    },
    {
        "id": "nippon-small-cap",
        "name": "Nippon India Small Cap Fund - Direct Plan - Growth",
        "short_name": "Nippon India Small Cap",
        "amc": "Nippon India Mutual Fund",
        "category": "Small Cap",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 172.30,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.72,
        "min_sip": 1000,
        "returns_1y": 42.8,
        "returns_3y": 31.5,
        "returns_5y": 32.1,
        "returns_10y": 22.4,
        "benchmark": "NIFTY Smallcap 250 TRI",
        "risk": "Very High",
        "fund_manager": "Samir Rachh",
        "aum_cr": 58400.0,
        "rating": 5,
        "top_holdings": ["Tube Investments", "HDFC Bank", "Apar Industries", "KPIT Tech", "Tejas Networks"],
        "description": "Highly diversified small cap portfolio identifying emerging champions in niche manufacturing and technology."
    },

    # ── EQUITY: ELSS TAX SAVER (SECTION 80C) ───────────────────────────────────
    {
        "id": "mirae-elss-tax",
        "name": "Mirae Asset ELSS Tax Saver Fund - Direct Plan - Growth",
        "short_name": "Mirae Asset ELSS Tax Saver",
        "amc": "Mirae Asset Mutual Fund",
        "category": "ELSS (Tax Saving)",
        "asset_class": "EQUITY",
        "plan": "Direct",
        "option": "Growth",
        "nav": 49.80,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.58,
        "min_sip": 500,
        "returns_1y": 29.5,
        "returns_3y": 19.8,
        "returns_5y": 20.4,
        "returns_10y": 18.2,
        "benchmark": "NIFTY 500 TRI",
        "risk": "Very High",
        "fund_manager": "Neelesh Surana",
        "aum_cr": 24100.0,
        "rating": 5,
        "top_holdings": ["HDFC Bank", "ICICI Bank", "Reliance", "Infosys", "Axis Bank"],
        "description": "Tax-saving equity fund eligible for Section 80C deductions (up to ₹1.5L/year) with mandatory 3-year lock-in."
    },

    # ── HYBRID: BALANCED ADVANTAGE & MULTI ASSET ───────────────────────────────
    {
        "id": "hdfc-balanced-advantage",
        "name": "HDFC Balanced Advantage Fund - Direct Plan - Growth",
        "short_name": "HDFC Balanced Advantage",
        "amc": "HDFC Mutual Fund",
        "category": "Balanced Advantage",
        "asset_class": "HYBRID",
        "plan": "Direct",
        "option": "Growth",
        "nav": 512.40,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.74,
        "min_sip": 500,
        "returns_1y": 24.8,
        "returns_3y": 20.2,
        "returns_5y": 18.9,
        "returns_10y": 14.5,
        "benchmark": "NIFTY 50 Hybrid Composite Debt 50:50",
        "risk": "High",
        "fund_manager": "Gopal Agrawal & Srinivasan Ramamurthy",
        "aum_cr": 92300.0,
        "rating": 5,
        "top_holdings": ["HDFC Bank", "ICICI Bank", "Coal India", "Infosys", "GOI 7.18% 2033 Bond"],
        "description": "Dynamically manages equity and debt allocation between 30% and 80% to protect downside during volatile markets."
    },
    {
        "id": "icici-multi-asset",
        "name": "ICICI Prudential Multi-Asset Fund - Direct Plan - Growth",
        "short_name": "ICICI Pru Multi-Asset",
        "amc": "ICICI Prudential Mutual Fund",
        "category": "Multi Asset Allocation",
        "asset_class": "HYBRID",
        "plan": "Direct",
        "option": "Growth",
        "nav": 684.20,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.79,
        "min_sip": 500,
        "returns_1y": 27.6,
        "returns_3y": 22.4,
        "returns_5y": 21.0,
        "returns_10y": 16.2,
        "benchmark": "Custom Multi-Asset Benchmark",
        "risk": "Very High",
        "fund_manager": "Sankaran Naren",
        "aum_cr": 51200.0,
        "rating": 5,
        "top_holdings": ["ICICI Bank", "NTPC", "Bharti Airtel", "Physical Gold", "GOI Sovereign Bonds"],
        "description": "Multi-asset diversification investing across domestic equity (50-60%), debt securities (15-25%), and gold/silver bullion (10-15%)."
    },

    # ── DEBT: LIQUID & CORPORATE BOND ──────────────────────────────────────────
    {
        "id": "hdfc-liquid-fund",
        "name": "HDFC Liquid Fund - Direct Plan - Growth",
        "short_name": "HDFC Liquid Fund",
        "amc": "HDFC Mutual Fund",
        "category": "Liquid",
        "asset_class": "DEBT",
        "plan": "Direct",
        "option": "Growth",
        "nav": 4725.80,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.20,
        "min_sip": 500,
        "returns_1y": 7.25,
        "returns_3y": 6.85,
        "returns_5y": 5.95,
        "returns_10y": 6.60,
        "benchmark": "CRISIL Liquid Debt Index",
        "risk": "Low to Moderate",
        "fund_manager": "Anupam Joshi",
        "aum_cr": 68400.0,
        "rating": 4,
        "top_holdings": ["Treasury Bills", "Certificate of Deposit", "Commercial Papers"],
        "description": "High-safety liquid fund for parking emergency funds and short-term surplus with T+1 instant liquidity."
    },
    {
        "id": "sbi-corporate-bond",
        "name": "SBI Corporate Bond Fund - Direct Plan - Growth",
        "short_name": "SBI Corporate Bond",
        "amc": "SBI Mutual Fund",
        "category": "Corporate Bond",
        "asset_class": "DEBT",
        "plan": "Direct",
        "option": "Growth",
        "nav": 42.15,
        "nav_date": "2026-11-14",
        "expense_ratio": 0.34,
        "min_sip": 500,
        "returns_1y": 8.10,
        "returns_3y": 7.30,
        "returns_5y": 6.85,
        "returns_10y": 7.45,
        "benchmark": "CRISIL Corporate Bond Composite",
        "risk": "Moderate",
        "fund_manager": "Rajeev Radhakrishnan",
        "aum_cr": 21800.0,
        "rating": 4,
        "top_holdings": ["REC Ltd AAA Bonds", "NABARD AAA", "PFC AAA", "HDFC Bank AAA"],
        "description": "High credit-quality debt portfolio investing >80% in top-rated AAA corporate bonds."
    }
]


def list_schemes(asset_class="ALL", category="ALL", search="", sort_by="returns_3y", page=1, limit=12):
    """Filter, sort, and paginate mutual fund schemes."""
    results = SCHEMES_CATALOG

    if asset_class and asset_class.upper() != "ALL":
        results = [x for x in results if x["asset_class"] == asset_class.upper()]

    if category and category.upper() != "ALL":
        c_up = category.upper()
        results = [x for x in results if c_up in x["category"].upper()]

    if search:
        s = search.strip().upper()
        results = [
            x for x in results
            if (s in x["name"].upper() or
                s in x["amc"].upper() or
                s in x["category"].upper() or
                s in x.get("fund_manager", "").upper())
        ]

    # Sort
    valid_sorts = {
        "returns_1y": lambda x: x.get("returns_1y", 0.0),
        "returns_3y": lambda x: x.get("returns_3y", 0.0),
        "returns_5y": lambda x: x.get("returns_5y", 0.0),
        "aum": lambda x: x.get("aum_cr", 0.0),
        "expense_ratio": lambda x: -x.get("expense_ratio", 99.0)  # lower is better
    }
    sort_fn = valid_sorts.get(sort_by, valid_sorts["returns_3y"])
    results = sorted(results, key=sort_fn, reverse=True)

    total = len(results)
    start = (page - 1) * limit
    end = start + limit
    paginated = results[start:end]

    return {
        "schemes": paginated,
        "total": total,
        "page": page,
        "limit": limit,
        "totalPages": max(1, (total + limit - 1) // limit)
    }


def get_scheme_by_id(scheme_id: str):
    """Retrieve full details of a specific mutual fund scheme."""
    sid = scheme_id.strip().lower()
    for s in SCHEMES_CATALOG:
        if s["id"].lower() == sid:
            return s
    return None


def calculate_sip(monthly_amount: float, annual_return_pct: float, duration_years: int, step_up_pct: float = 0.0) -> dict:
    """
    Compute compound SIP investment with optional annual step-up.
    Mathematically rigorous monthly compounding simulation.
    """
    P = float(monthly_amount)
    r = float(annual_return_pct) / 100.0
    i = r / 12.0  # monthly interest rate
    step_up = float(step_up_pct) / 100.0
    total_months = duration_years * 12

    current_sip = P
    total_invested = 0.0
    corpus = 0.0
    year_wise = []

    for month in range(1, total_months + 1):
        # Step-up applies at the start of each new year (month 13, 25, 37...)
        if month > 1 and (month - 1) % 12 == 0 and step_up > 0:
            current_sip = current_sip * (1.0 + step_up)

        # Deposit at beginning of month, compounding for the month
        total_invested += current_sip
        corpus = (corpus + current_sip) * (1.0 + i)

        # Record end of year milestones
        if month % 12 == 0:
            yr = month // 12
            gain = max(0.0, corpus - total_invested)
            year_wise.append({
                "year": yr,
                "monthly_sip": round(current_sip, 2),
                "total_invested": round(total_invested, 2),
                "future_value": round(corpus, 2),
                "wealth_gain": round(gain, 2)
            })

    total_gain = max(0.0, corpus - total_invested)
    multiplier = round(corpus / total_invested, 2) if total_invested > 0 else 1.0

    return {
        "monthly_investment": P,
        "annual_return_pct": annual_return_pct,
        "duration_years": duration_years,
        "step_up_pct": step_up_pct,
        "total_invested": round(total_invested, 2),
        "future_value": round(corpus, 2),
        "wealth_gain": round(total_gain, 2),
        "wealth_multiplier": multiplier,
        "year_wise_breakdown": year_wise
    }


def generate_sip_recommendations(
    monthly_budget: float,
    horizon_years: int,
    risk_tolerance: str = "MODERATE",
    financial_goal: str = "WEALTH_CREATION",
    tax_saving: bool = False
) -> dict:
    """
    Transparent rule-based advisory engine generating targeted asset allocations
    and specific mutual fund scheme picks.
    """
    budget = float(monthly_budget)
    risk = risk_tolerance.upper().strip()
    goal = financial_goal.upper().strip()
    years = int(horizon_years)

    allocations = []
    recommended_schemes = []
    rationale = []

    # Case A: Tax Saving Goal (Section 80C)
    if tax_saving or goal == "TAX_SAVING":
        rationale.append("Prioritizing Section 80C tax-saving ELSS funds (up to ₹1.5L annual deduction with 3-year lock-in).")
        elss_alloc = 50.0
        flexi_alloc = 30.0
        index_alloc = 20.0
        allocations = [
            {"category": "ELSS Tax Saver", "pct": elss_alloc, "amount": round(budget * 0.50, 2)},
            {"category": "Flexi Cap", "pct": flexi_alloc, "amount": round(budget * 0.30, 2)},
            {"category": "Nifty 50 Index", "pct": index_alloc, "amount": round(budget * 0.20, 2)}
        ]
        recommended_schemes = [
            get_scheme_by_id("mirae-elss-tax"),
            get_scheme_by_id("ppfas-flexi-cap"),
            get_scheme_by_id("uti-nifty-50-index")
        ]

    # Case B: Short Horizon (< 3 years) or Conservative Risk
    elif years < 3 or risk == "CONSERVATIVE":
        rationale.append("Short investment horizon or conservative profile: prioritizing capital preservation and low volatility.")
        allocations = [
            {"category": "Liquid & Ultra Short Debt", "pct": 40.0, "amount": round(budget * 0.40, 2)},
            {"category": "Balanced Advantage (Dynamic)", "pct": 40.0, "amount": round(budget * 0.40, 2)},
            {"category": "Corporate Bond (AAA)", "pct": 20.0, "amount": round(budget * 0.20, 2)}
        ]
        recommended_schemes = [
            get_scheme_by_id("hdfc-liquid-fund"),
            get_scheme_by_id("hdfc-balanced-advantage"),
            get_scheme_by_id("sbi-corporate-bond")
        ]

    # Case C: Medium Horizon (3 to 7 years) and Moderate Risk
    elif years <= 7 or risk == "MODERATE":
        rationale.append("Balanced medium-term horizon: combining large-cap stability with flexi-cap compounding.")
        allocations = [
            {"category": "Flexi Cap Equity", "pct": 40.0, "amount": round(budget * 0.40, 2)},
            {"category": "Nifty 50 Index (Large Cap)", "pct": 30.0, "amount": round(budget * 0.30, 2)},
            {"category": "Balanced Advantage / Multi-Asset", "pct": 30.0, "amount": round(budget * 0.30, 2)}
        ]
        recommended_schemes = [
            get_scheme_by_id("ppfas-flexi-cap"),
            get_scheme_by_id("uti-nifty-50-index"),
            get_scheme_by_id("hdfc-balanced-advantage")
        ]

    # Case D: Long Horizon (7+ years) and Aggressive Wealth Creation
    else:
        rationale.append("Long-term aggressive wealth creation: maximizing multi-cap compounding with focused midcap & smallcap allocation.")
        allocations = [
            {"category": "Flexi Cap / Multi Cap", "pct": 40.0, "amount": round(budget * 0.40, 2)},
            {"category": "Mid Cap Champions", "pct": 25.0, "amount": round(budget * 0.25, 2)},
            {"category": "Small Cap High Growth", "pct": 20.0, "amount": round(budget * 0.20, 2)},
            {"category": "Nifty 50 Index", "pct": 15.0, "amount": round(budget * 0.15, 2)}
        ]
        recommended_schemes = [
            get_scheme_by_id("ppfas-flexi-cap"),
            get_scheme_by_id("motilal-midcap"),
            get_scheme_by_id("nippon-small-cap"),
            get_scheme_by_id("uti-nifty-50-index")
        ]

    # Calculate 10-year projected return for the recommendation
    weighted_cagr = 14.5 if risk == "AGGRESSIVE" else (11.5 if risk == "MODERATE" else 8.5)
    projection = calculate_sip(budget, weighted_cagr, max(5, years), step_up_pct=5.0)

    return {
        "monthly_budget": budget,
        "horizon_years": years,
        "risk_tolerance": risk,
        "financial_goal": goal,
        "tax_saving": tax_saving,
        "expected_portfolio_cagr": weighted_cagr,
        "allocations": allocations,
        "recommended_schemes": [s for s in recommended_schemes if s is not None],
        "rationale": " ".join(rationale),
        "ten_year_projection": projection
    }


def compare_schemes(scheme_ids: list) -> list:
    """Compare 2-4 mutual fund schemes side by side."""
    selected = []
    for sid in scheme_ids:
        s = get_scheme_by_id(sid)
        if s:
            selected.append(s)
    return selected

