"""
InvestSphere Financial Market News Service
Provides structured, categorized financial articles for Indian stock markets, economy, IPOs, mutual funds, and global trends.
Features category filtering, search, pagination, and multi-source aggregation.
"""

import datetime

CATEGORIES = [
    {"code": "ALL",            "name": "All Financial News",        "icon": "📰"},
    {"code": "MARKET",         "name": "Indian Stock Market",       "icon": "📈"},
    {"code": "INDICES",        "name": "NSE & BSE (Nifty/Sensex)",  "icon": "📊"},
    {"code": "BANKING",        "name": "Banking & Finance",         "icon": "🏦"},
    {"code": "ECONOMY",        "name": "Economy & Policy",          "icon": "🏛️"},
    {"code": "IPO",            "name": "IPO News & Filings",        "icon": "🚀"},
    {"code": "COMPANIES",      "name": "Company News & Earnings",   "icon": "🏢"},
    {"code": "GLOBAL",         "name": "Global Markets",            "icon": "🌍"},
    {"code": "COMMODITIES",    "name": "Commodities & Gold",        "icon": "🏆"},
    {"code": "MUTUAL_FUNDS",   "name": "Mutual Funds & SIP",        "icon": "🔄"},
    {"code": "TECH",           "name": "Technology Stocks",         "icon": "💻"},
]

ARTICLES_DATA = [
    {
        "id": "news-101",
        "headline": "NTPC Green Energy IPO Opens Nov 19: Price Band Fixed at ₹102-108; All You Need to Know",
        "source": "The Economic Times",
        "published_at": "2026-11-15T08:30:00Z",
        "category": "IPO",
        "category_name": "IPO News & Filings",
        "summary": "State-owned NTPC Green Energy's ₹10,000-crore initial public offering is set to open for public subscription from November 19 to 22. The price band has been set at ₹102 to ₹108 per equity share.",
        "url": "https://economictimes.indiatimes.com/markets/ipos/fpos/ntpc-green-energy-ipo-price-band-dates-lot-size",
        "thumbnail": "https://images.unsplash.com/photo-1466611653911-95081537e5b7?w=600&auto=format&fit=crop&q=80",
        "author": "ET Bureau"
    },
    {
        "id": "news-102",
        "headline": "Nifty 50 Defends 24,500 Support; FII Outflows Slow Ahead of Key Macro Releases",
        "source": "Moneycontrol",
        "published_at": "2026-11-15T09:15:00Z",
        "category": "INDICES",
        "category_name": "NSE & BSE (Nifty/Sensex)",
        "summary": "Benchmark indices witnessed strong resilience around 24,500 levels as domestic institutional investors (DIIs) provided counterbalancing liquidity against foreign portfolio outflows.",
        "url": "https://www.moneycontrol.com/news/business/markets/market-wrap-nifty-sensex-fii-dii-activity",
        "thumbnail": "https://images.unsplash.com/photo-1611974789855-9c2a0a7236a3?w=600&auto=format&fit=crop&q=80",
        "author": "Markets Desk"
    },
    {
        "id": "news-103",
        "headline": "RBI Governor Reaffirms 4% Inflation Target; Signals Measured Liquidity Stance",
        "source": "Livemint",
        "published_at": "2026-11-14T17:45:00Z",
        "category": "ECONOMY",
        "category_name": "Economy & Policy",
        "summary": "Reserve Bank of India reiterated its steadfast commitment to durably aligning consumer price inflation with the 4 percent target while supporting high GDP growth trajectory in urban and rural hubs.",
        "url": "https://www.livemint.com/economy/rbi-mpc-inflation-gdp-monetary-policy-announcement",
        "thumbnail": "https://images.unsplash.com/photo-1526304640581-d334cdbbf45e?w=600&auto=format&fit=crop&q=80",
        "author": "Ananya Roy"
    },
    {
        "id": "news-104",
        "headline": "HDFC Bank & ICICI Bank Lead Private Credit Growth with Stable Net Interest Margins",
        "source": "Business Standard",
        "published_at": "2026-11-14T14:10:00Z",
        "category": "BANKING",
        "category_name": "Banking & Finance",
        "summary": "Indian top-tier private lenders posted robust deposit accretion and loan expansion in retail, MSME, and infrastructure credit segments, keeping gross NPAs at multi-year record lows.",
        "url": "https://www.business-standard.com/finance/banking/private-banks-hdfc-icici-credit-growth",
        "thumbnail": "https://images.unsplash.com/photo-1541354329998-f4d9a9f9297f?w=600&auto=format&fit=crop&q=80",
        "author": "Banking Bureau"
    },
    {
        "id": "news-105",
        "headline": "Swiggy Shares Extend Gains to Touch ₹456 Post Debut; Brokerages Initiate Outperform",
        "source": "CNBC-TV18",
        "published_at": "2026-11-14T11:20:00Z",
        "category": "COMPANIES",
        "category_name": "Company News & Earnings",
        "summary": "Quick-commerce and food delivery giant Swiggy shares climbed over 16 percent above its issue price of ₹390 as institutional brokerages projected rapid EBITDA profitability across Instamart fulfillment dark stores.",
        "url": "https://www.cnbctv18.com/market/stocks/swiggy-share-price-listing-gains-target-price-instamart",
        "thumbnail": "https://images.unsplash.com/photo-1526367790999-0150786686a2?w=600&auto=format&fit=crop&q=80",
        "author": "Tech & Equities Team"
    },
    {
        "id": "news-106",
        "headline": "Gold Consolidation at ₹76,200 per 10g in India; Silver Demand Surges on Industrial Green Tech",
        "source": "Reuters",
        "published_at": "2026-11-14T10:00:00Z",
        "category": "COMMODITIES",
        "category_name": "Commodities & Gold",
        "summary": "Gold traded in a consolidated band on MCX as bullion buyers observed wedding season momentum in Maharashtra and southern retail markets. Silver held near ₹91,500/kg backed by photovoltaic solar cell manufacturing demand.",
        "url": "https://www.reuters.com/markets/commodities/gold-silver-prices-india-mcx-global-rates",
        "thumbnail": "https://images.unsplash.com/photo-1610375461246-83df859d849d?w=600&auto=format&fit=crop&q=80",
        "author": "Commodities Desk"
    },
    {
        "id": "news-107",
        "headline": "Mutual Fund Inflows Hit Record ₹25,000 Crore Monthly SIP Milestone: AMFI Data",
        "source": "The Hindu BusinessLine",
        "published_at": "2026-11-13T16:30:00Z",
        "category": "MUTUAL_FUNDS",
        "category_name": "Mutual Funds & SIP",
        "summary": "Systematic Investment Plans (SIPs) from retail Indian investors crossed unprecedented monthly inflows of ₹25,322 crore, demonstrating the sustained structural shift toward long-term disciplined equity compounding.",
        "url": "https://www.thehindubusinessline.com/markets/amfi-mutual-fund-sip-inflows-record-equity-schemes",
        "thumbnail": "https://images.unsplash.com/photo-1559526324-4b87b5e36e44?w=600&auto=format&fit=crop&q=80",
        "author": "MF Research Desk"
    },
    {
        "id": "news-108",
        "headline": "Indian IT Services Giants Accelerate Generative AI Deal Pipelines; Cloud Migrations Steady",
        "source": "Mint",
        "published_at": "2026-11-13T13:40:00Z",
        "category": "TECH",
        "category_name": "Technology Stocks",
        "summary": "TCS, Infosys, and HCL Tech expanded client engagements in enterprise Generative AI co-pilot systems and cloud modernization architectures across North American and European BFSI verticals.",
        "url": "https://www.livemint.com/technology/tech-news/it-services-genai-deals-tcs-infosys-wipro",
        "thumbnail": "https://images.unsplash.com/photo-1518770660439-4636190af475?w=600&auto=format&fit=crop&q=80",
        "author": "Tech Bureau"
    },
    {
        "id": "news-109",
        "headline": "Global Markets Rally as Fed Holds Rates Steady; US Dollar Index Moderates",
        "source": "Bloomberg",
        "published_at": "2026-11-13T09:00:00Z",
        "category": "GLOBAL",
        "category_name": "Global Markets",
        "summary": "Wall Street indices S&P 500 and Nasdaq ended higher while Asian equities gained on easing US Treasury yields and optimistic corporate earnings reports across semiconductors and consumer sectors.",
        "url": "https://www.bloomberg.com/markets/global-equities-fed-treasury-yields",
        "thumbnail": "https://images.unsplash.com/photo-1486406146926-c627a92ad1ab?w=600&auto=format&fit=crop&q=80",
        "author": "Global Markets Team"
    },
    {
        "id": "news-110",
        "headline": "Domestic Stock Market Outlook: Key Factors to Watch in Coming Trading Sessions",
        "source": "The Economic Times",
        "published_at": "2026-11-12T18:00:00Z",
        "category": "MARKET",
        "category_name": "Indian Stock Market",
        "summary": "Technical analysts emphasize strong sectoral rotation into Capital Goods, Defense, and Pharma segments. Volatility indices remain subdued as corporate earnings match quarterly expectations.",
        "url": "https://economictimes.indiatimes.com/markets/stocks/news/market-outlook-key-triggers-nifty-bank-nifty",
        "thumbnail": "https://images.unsplash.com/photo-1590283603385-17ffb3a7f29f?w=600&auto=format&fit=crop&q=80",
        "author": "ET Markets"
    },
    {
        "id": "news-111",
        "headline": "SEBI Streamlines Rights Issue Process; Cuts Timeline to Faster 23 Days",
        "source": "Business Standard",
        "published_at": "2026-11-15T11:00:00Z",
        "category": "ECONOMY",
        "category_name": "Economy & Policy",
        "summary": "Capital markets regulator SEBI approved major reforms to shorten the turnaround time for corporate rights issues, boosting market liquidity and making capital raising more accessible for Indian enterprises.",
        "url": "https://www.business-standard.com/markets/news/sebi-board-approves-faster-rights-issue-timeline-regulations",
        "thumbnail": "https://images.unsplash.com/photo-1486406146926-c627a92ad1ab?w=600&auto=format&fit=crop&q=80",
        "author": "BS Reporter"
    }
]


def list_news(category="ALL", search="", page=1, limit=10):
    """Filter, search, and paginate news articles."""
    results = ARTICLES_DATA

    # Filter by category
    if category and category.upper() != "ALL":
        cat_upper = category.upper()
        # Support aliases
        if cat_upper in ("STOCK", "STOCKS"):
            cat_upper = "MARKET"
        results = [x for x in results if x["category"] == cat_upper]

    # Search filter
    if search:
        s = search.strip().upper()
        results = [
            x for x in results
            if s in x["headline"].upper() or s in x["summary"].upper() or s in x["source"].upper()
        ]

    total = len(results)
    start = (page - 1) * limit
    end = start + limit
    paginated = results[start:end]

    return {
        "articles": paginated,
        "total": total,
        "page": page,
        "limit": limit,
        "totalPages": max(1, (total + limit - 1) // limit),
        "category": category.upper()
    }


def get_categories():
    """Retrieve list of news categories with article counts."""
    counts = {}
    for a in ARTICLES_DATA:
        c = a["category"]
        counts[c] = counts.get(c, 0) + 1

    cats_with_counts = []
    for c in CATEGORIES:
        cnt = len(ARTICLES_DATA) if c["code"] == "ALL" else counts.get(c["code"], 0)
        cats_with_counts.append({
            "code": c["code"],
            "name": c["name"],
            "icon": c["icon"],
            "count": cnt
        })
    return cats_with_counts
