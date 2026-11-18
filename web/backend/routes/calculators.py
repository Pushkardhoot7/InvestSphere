"""
InvestSphere Financial Calculators Routes
SIP and SWP calculations — Python mirrors of C++ AnnuityEngine
"""

import math
from flask import Blueprint, request
from helpers import require_auth, success, error

calculators_bp = Blueprint("calculators", __name__)


# ─────────────────────────────────────────────────────────────────────────────
# SIP Calculator
# Standard end-of-month ordinary annuity with optional annual step-up
# ─────────────────────────────────────────────────────────────────────────────

@calculators_bp.route("/sip", methods=["POST"])
@require_auth
def calculate_sip():
    data = request.get_json(silent=True) or {}

    try:
        monthly = float(data.get("monthly_amount") or 0)
        rate = float(data.get("annual_return_percent") or 0)
        years = int(data.get("duration_years") or 0)
        step_up = float(data.get("annual_step_up_percent") or 0)
    except (ValueError, TypeError):
        return error("Invalid numeric input.")

    if monthly < 1:
        return error("Monthly investment amount must be at least ₹1.")
    if not (0 <= rate <= 100):
        return error("Expected return rate must be between 0% and 100%.")
    if not (1 <= years <= 50):
        return error("Investment duration must be between 1 and 50 years.")
    if not (0 <= step_up <= 100):
        return error("Annual step-up percentage must be between 0% and 100%.")

    r = rate / 1200.0  # monthly rate
    zero_rate = r < 1e-9

    running_corpus = 0.0
    total_invested = 0.0
    current_monthly = monthly
    yearly_breakdown = []

    for y in range(1, years + 1):
        # Grow existing corpus for 12 months
        growth_factor = 1.0 if zero_rate else math.pow(1.0 + r, 12)
        grown_corpus = running_corpus * growth_factor

        # FV of this year's 12 monthly contributions (ordinary annuity)
        if zero_rate:
            year_fv = current_monthly * 12.0
        else:
            year_fv = current_monthly * ((math.pow(1.0 + r, 12) - 1.0) / r)

        running_corpus = grown_corpus + year_fv
        total_invested += current_monthly * 12.0

        yearly_breakdown.append({
            "year": y,
            "invested_at_year_end": round(total_invested, 2),
            "portfolio_value_at_year_end": round(running_corpus, 2),
            "wealth_gain": round(running_corpus - total_invested, 2),
            "return_percent": round(
                ((running_corpus - total_invested) / total_invested * 100) if total_invested > 0 else 0, 2
            ),
        })

        if step_up > 0:
            current_monthly *= (1.0 + step_up / 100.0)

    result = {
        "monthly_amount_initial": monthly,
        "annual_step_up_percent": step_up,
        "annual_return_percent": rate,
        "duration_years": years,
        "total_installments": years * 12,
        "total_invested": round(total_invested, 2),
        "maturity_value": round(running_corpus, 2),
        "estimated_gain": round(running_corpus - total_invested, 2),
        "return_percent": round(
            ((running_corpus - total_invested) / total_invested * 100) if total_invested > 0 else 0, 2
        ),
        "yearly_breakdown": yearly_breakdown,
    }
    return success(result)


# ─────────────────────────────────────────────────────────────────────────────
# SWP Calculator
# Growth-first then withdrawal each month
# ─────────────────────────────────────────────────────────────────────────────

@calculators_bp.route("/swp", methods=["POST"])
@require_auth
def calculate_swp():
    data = request.get_json(silent=True) or {}

    try:
        corpus = float(data.get("initial_corpus") or 0)
        withdrawal = float(data.get("monthly_withdrawal") or 0)
        rate = float(data.get("annual_return_percent") or data.get("annual_rate") or 0)
        years = int(data.get("duration_years") or data.get("tenure_years") or 0)
    except (ValueError, TypeError):
        return error("Invalid numeric input.")

    if corpus < 1000:
        return error("Initial corpus must be at least ₹1,000.")
    if withdrawal < 0:
        return error("Monthly withdrawal cannot be negative.")
    if not (0 <= rate <= 100):
        return error("Expected return rate must be between 0% and 100%.")
    if not (1 <= years <= 50):
        return error("Withdrawal duration must be between 1 and 50 years.")

    r = rate / 1200.0
    zero_rate = r < 1e-9
    total_months = years * 12

    balance = corpus
    total_withdrawn = 0.0
    total_interest = 0.0
    depleted = False
    depleted_month = 0
    withdrawals_completed = 0
    yearly_breakdown = []

    for y in range(1, years + 1):
        year_withdrawn = 0.0
        year_depleted = False
        year_opening_balance = balance
        year_interest = 0.0

        for m in range(1, 13):
            if depleted:
                break
            global_month = (y - 1) * 12 + m
            if global_month > total_months:
                break

            # 1. Growth phase
            interest = 0.0 if zero_rate else balance * r
            total_interest += interest
            year_interest += interest
            balance += interest

            # 2. Withdrawal phase
            if withdrawal <= 0:
                pass
            elif balance > withdrawal + 1e-9:
                balance -= withdrawal
                total_withdrawn += withdrawal
                year_withdrawn += withdrawal
                withdrawals_completed += 1
            elif abs(balance - withdrawal) <= 1e-9:
                total_withdrawn += withdrawal
                year_withdrawn += withdrawal
                withdrawals_completed += 1
                balance = 0.0
                if global_month < total_months:
                    depleted = True
                    depleted_month = global_month
                    year_depleted = True
                    break
            elif balance > 1e-9:
                total_withdrawn += balance
                year_withdrawn += balance
                withdrawals_completed += 1
                depleted = True
                depleted_month = global_month
                year_depleted = True
                balance = 0.0
                break
            else:
                depleted = True
                depleted_month = global_month
                year_depleted = True
                break

        yearly_breakdown.append({
            "year": y,
            "monthly_withdrawal": withdrawal,
            "total_withdrawn_this_year": round(year_withdrawn, 2),
            "balance_at_year_end": round(balance, 2),
            "opening_balance": round(year_opening_balance, 2),
            "interest_earned": round(year_interest, 2),
            "depleted_this_year": year_depleted,
            "depleted_at_month": depleted_month if year_depleted else 0,
        })

        if depleted:
            break

    result = {
        "initial_corpus": corpus,
        "monthly_withdrawal": withdrawal,
        "annual_return_percent": rate,
        "duration_years": years,
        "total_withdrawn": round(total_withdrawn, 2),
        "final_balance": round(balance, 2),
        "total_earnings": round(total_interest, 2),
        "withdrawals_completed": withdrawals_completed,
        "depleted": depleted,
        "is_depleted": depleted,
        "depleted_at_month": depleted_month,
        "depleted_at_year": ((depleted_month + 11) // 12) if depleted else 0,
        "depleted_in_year": ((depleted_month + 11) // 12) if depleted else 0,
        "yearly_breakdown": yearly_breakdown,
        "schedule": yearly_breakdown,
    }
    return success(result)


# ─────────────────────────────────────────────────────────────────────────────
# Fixed Deposit (FD) Calculator
# Accurate compounding (Monthly, Quarterly, Half-Yearly, Annually, Simple)
# ─────────────────────────────────────────────────────────────────────────────

@calculators_bp.route("/fd", methods=["POST"])
@require_auth
def calculate_fd():
    data = request.get_json(silent=True) or {}

    try:
        principal = float(data.get("principal") or data.get("principal_amount") or 0)
        rate = float(data.get("annual_interest_rate") or data.get("interest_rate") or 0)
        tenure_years = float(data.get("tenure_years") or 0)
        tenure_months = int(data.get("tenure_months") or 0)
        if tenure_months > 0 and tenure_years <= 0:
            tenure_years = tenure_months / 12.0
        elif tenure_years > 0 and tenure_months <= 0:
            tenure_months = int(round(tenure_years * 12))
    except (ValueError, TypeError):
        return error("Invalid numeric input.")

    if principal < 100:
        return error("Principal amount must be at least ₹100.")
    if not (0 <= rate <= 50):
        return error("Interest rate must be between 0% and 50% per annum.")
    if tenure_years <= 0 or tenure_years > 30:
        return error("Tenure must be between 1 month and 30 years.")

    comp_freq = str(data.get("compounding_frequency") or "QUARTERLY").upper().strip()
    payout_type = str(data.get("payout_type") or "CUMULATIVE").upper().strip()
    institution = str(data.get("institution_name") or "Bank").strip()

    freq_map = {
        "MONTHLY": 12,
        "QUARTERLY": 4,
        "HALF_YEARLY": 2,
        "ANNUALLY": 1,
        "SIMPLE": 0
    }
    n = freq_map.get(comp_freq, 4)
    r = rate / 100.0

    if n == 0 or comp_freq == "SIMPLE":
        # Simple Interest: A = P * (1 + r * t)
        total_interest = principal * r * tenure_years
        maturity_amount = principal + total_interest
        eff_yield = rate
        periodic_payout = (principal * r) / 12.0  # monthly equivalent
    else:
        # Compound Interest: A = P * (1 + r/n)^(n*t)
        total_periods = n * tenure_years
        maturity_amount = principal * math.pow(1.0 + r / n, total_periods)
        total_interest = maturity_amount - principal
        eff_yield = (math.pow(1.0 + r / n, n) - 1.0) * 100.0
        periodic_payout = (principal * r) / n

    if payout_type in ("PERIODIC_QUARTERLY", "PERIODIC_MONTHLY", "REGULAR_PAYOUT"):
        # Regular interest paid out per period, principal returned at maturity
        total_payout_received = principal * r * tenure_years
        closing_principal = principal
        display_maturity = principal
    else:
        # Cumulative (reinvested)
        total_payout_received = 0.0
        closing_principal = maturity_amount
        display_maturity = maturity_amount

    # Build yearly breakdown schedule
    yearly_schedule = []
    num_years = max(1, int(math.ceil(tenure_years)))
    running_balance = principal

    for y in range(1, num_years + 1):
        y_tenure = 1.0 if y < num_years else (tenure_years - (num_years - 1))
        if y_tenure <= 0:
            y_tenure = 1.0

        if n == 0 or comp_freq == "SIMPLE":
            y_interest = principal * r * y_tenure
            opening = principal
            closing = principal + (principal * r * min(float(y), tenure_years))
        elif payout_type in ("PERIODIC_QUARTERLY", "PERIODIC_MONTHLY", "REGULAR_PAYOUT"):
            y_interest = principal * r * y_tenure
            opening = principal
            closing = principal
        else:
            opening = running_balance
            closing = opening * math.pow(1.0 + r / n, n * y_tenure)
            y_interest = closing - opening
            running_balance = closing

        yearly_schedule.append({
            "year": y,
            "opening_balance": round(opening, 2),
            "interest_earned": round(y_interest, 2),
            "closing_balance": round(closing, 2),
            "cumulative_interest": round(closing - principal if payout_type == "CUMULATIVE" else principal * r * min(float(y), tenure_years), 2)
        })

    result = {
        "institution_name": institution,
        "principal_amount": round(principal, 2),
        "annual_interest_rate": round(rate, 2),
        "tenure_years": round(tenure_years, 2),
        "tenure_months": tenure_months,
        "compounding_frequency": comp_freq,
        "payout_type": payout_type,
        "total_interest_earned": round(total_interest, 2),
        "maturity_amount": round(display_maturity, 2),
        "estimated_maturity_amount": round(maturity_amount, 2),
        "total_payout_received": round(total_payout_received, 2),
        "effective_annual_yield_percent": round(eff_yield, 2),
        "periodic_payout_amount": round(periodic_payout, 2),
        "total_return_percent": round((total_interest / principal * 100.0) if principal > 0 else 0.0, 2),
        "yearly_schedule": yearly_schedule,
        "schedule": yearly_schedule
    }
    return success(result)


# ─────────────────────────────────────────────────────────────────────────────
# Bond & Debenture Calculator
# Yield to Maturity (YTM), Coupon Cashflows & Capital Gain/Loss
# ─────────────────────────────────────────────────────────────────────────────

@calculators_bp.route("/bond", methods=["POST"])
@require_auth
def calculate_bond():
    data = request.get_json(silent=True) or {}

    try:
        face_value = float(data.get("face_value") or 1000)
        purchase_price = float(data.get("purchase_price") or data.get("unit_price") or face_value)
        quantity = float(data.get("quantity") or data.get("qty") or 1)
        coupon_rate = float(data.get("coupon_rate") or 0)
        tenure_years = float(data.get("tenure_years") or 5)
    except (ValueError, TypeError):
        return error("Invalid numeric input.")

    if face_value <= 0:
        return error("Face value must be greater than zero.")
    if purchase_price <= 0:
        return error("Purchase price must be greater than zero.")
    if quantity <= 0:
        return error("Quantity must be at least 1 unit.")
    if not (0 <= coupon_rate <= 50):
        return error("Coupon rate must be between 0% and 50% per annum.")
    if not (0.25 <= tenure_years <= 50):
        return error("Tenure must be between 3 months and 50 years.")

    coupon_freq = str(data.get("coupon_frequency") or data.get("payment_frequency") or "SEMI_ANNUALLY").upper().strip()
    bond_name = str(data.get("bond_name") or data.get("name") or "Government / Corporate Bond").strip()
    symbol = str(data.get("symbol") or "BOND").strip().upper()

    freq_map = {
        "ANNUALLY": 1,
        "ANNUAL": 1,
        "SEMI_ANNUALLY": 2,
        "SEMI-ANNUAL": 2,
        "QUARTERLY": 4,
        "MONTHLY": 12
    }
    freq = freq_map.get(coupon_freq, 2)

    total_investment = purchase_price * quantity
    total_face_value = face_value * quantity

    # Annual coupon per bond = face_value * (coupon_rate / 100)
    annual_coupon_per_unit = face_value * (coupon_rate / 100.0)
    periodic_coupon_payout = (annual_coupon_per_unit * quantity) / freq
    total_periods = int(round(tenure_years * freq))
    total_coupon_interest = periodic_coupon_payout * total_periods

    # Capital gain/loss at maturity (Redemption price = face value)
    capital_gain_loss = total_face_value - total_investment
    total_net_return = total_coupon_interest + capital_gain_loss
    total_return_pct = (total_net_return / total_investment * 100.0) if total_investment > 0 else 0.0

    # Current Yield = (Annual Coupon / Purchase Price) * 100
    current_yield = (annual_coupon_per_unit / purchase_price * 100.0) if purchase_price > 0 else 0.0

    # Approx Yield to Maturity (YTM):
    # YTM = [C + (F - P) / n] / [(F + P) / 2]
    # where C = annual coupon, F = face value, P = purchase price, n = tenure
    avg_price = (face_value + purchase_price) / 2.0
    annual_capital_gain = (face_value - purchase_price) / tenure_years
    approx_ytm = ((annual_coupon_per_unit + annual_capital_gain) / avg_price * 100.0) if avg_price > 0 else current_yield

    # Yearly cash flow schedule
    yearly_schedule = []
    num_years = max(1, int(math.ceil(tenure_years)))
    cumulative_cash = 0.0

    for y in range(1, num_years + 1):
        is_final_year = (y == num_years)
        y_coupon = annual_coupon_per_unit * quantity
        principal_redemption = total_face_value if is_final_year else 0.0
        net_cashflow = y_coupon + principal_redemption
        cumulative_cash += net_cashflow

        yearly_schedule.append({
            "year": y,
            "annual_coupon": round(y_coupon, 2),
            "principal_redemption": round(principal_redemption, 2),
            "net_cashflow": round(net_cashflow, 2),
            "cumulative_cashflow": round(cumulative_cash, 2)
        })

    result = {
        "symbol": symbol,
        "bond_name": bond_name,
        "face_value": round(face_value, 2),
        "purchase_price": round(purchase_price, 2),
        "quantity": quantity,
        "coupon_rate": round(coupon_rate, 2),
        "coupon_frequency": coupon_freq,
        "tenure_years": round(tenure_years, 2),
        "total_investment": round(total_investment, 2),
        "total_invested": round(total_investment, 2),
        "total_face_value": round(total_face_value, 2),
        "maturity_value": round(total_face_value, 2),
        "periodic_coupon_payout": round(periodic_coupon_payout, 2),
        "total_coupon_interest": round(total_coupon_interest, 2),
        "capital_gain_loss": round(capital_gain_loss, 2),
        "total_net_return": round(total_net_return, 2),
        "total_return_percent": round(total_return_pct, 2),
        "current_yield_percent": round(current_yield, 2),
        "approx_ytm_percent": round(approx_ytm, 2),
        "yearly_schedule": yearly_schedule,
        "schedule": yearly_schedule
    }
    return success(result)


# ─────────────────────────────────────────────────────────────────────────────
# Gold & Precious Metals Investment Calculator
# Comprehensive bullion/jewellery estimation with charges, GST & appreciation
# ─────────────────────────────────────────────────────────────────────────────

@calculators_bp.route("/gold", methods=["POST"])
@require_auth
def calculate_gold():
    data = request.get_json(silent=True) or {}

    try:
        weight_grams = float(data.get("weight_grams") or data.get("weight") or 0)
        purity_karat = int(data.get("purity_karat") or data.get("purity") or 24)
        rate_per_gram = float(data.get("rate_per_gram") or 0)
        making_charge_pct = float(data.get("making_charge_percent") or data.get("making_charge_pct") or 0)
        gst_pct = float(data.get("gst_percent") or 3.0)
        hallmarking = float(data.get("hallmarking_charge") or 0)
        tenure_years = float(data.get("tenure_years") or 5)
        appreciation_rate = float(data.get("expected_annual_appreciation_percent") or 10.0)
    except (ValueError, TypeError):
        return error("Invalid numeric input.")

    if weight_grams <= 0 or weight_grams > 10000:
        return error("Gold weight must be between 0.1g and 10,000g.")
    if purity_karat not in (24, 22, 18, 14):
        return error("Purity must be 24K, 22K, 18K, or 14K.")
    if not (0 <= making_charge_pct <= 50):
        return error("Making charges must be between 0% and 50%.")
    if not (0 <= gst_pct <= 28):
        return error("GST rate must be between 0% and 28%.")

    # If rate_per_gram not provided, fetch live OroPocket rate
    base_24k_rate = rate_per_gram
    if base_24k_rate <= 0:
        try:
            from services.precious_metals_service import get_oropocket_rates
            rates = get_oropocket_rates()
            base_24k_rate = float(rates.get("gold", {}).get("sell_inr_per_gram") or 7800.0)
        except Exception:
            base_24k_rate = 7800.0

    purity_factor = purity_karat / 24.0
    effective_rate = base_24k_rate * purity_factor

    pure_metal_cost = weight_grams * effective_rate
    making_amount = pure_metal_cost * (making_charge_pct / 100.0)
    subtotal = pure_metal_cost + making_amount
    gst_amount = subtotal * (gst_pct / 100.0)
    total_acquisition_cost = subtotal + gst_amount + hallmarking

    # Projected appreciation
    projected_future_metal_value = pure_metal_cost * math.pow(1.0 + appreciation_rate / 100.0, tenure_years)
    projected_gain = projected_future_metal_value - total_acquisition_cost
    projected_return_pct = (projected_gain / total_acquisition_cost * 100.0) if total_acquisition_cost > 0 else 0.0

    # Year-by-year appreciation schedule
    yearly_schedule = []
    num_years = max(1, int(math.ceil(tenure_years)))
    for y in range(1, num_years + 1):
        y_val = pure_metal_cost * math.pow(1.0 + appreciation_rate / 100.0, y)
        yearly_schedule.append({
            "year": y,
            "projected_metal_value": round(y_val, 2),
            "net_gain": round(y_val - total_acquisition_cost, 2),
            "return_percent": round(((y_val - total_acquisition_cost) / total_acquisition_cost * 100.0) if total_acquisition_cost > 0 else 0.0, 2)
        })

    result = {
        "weight_grams": round(weight_grams, 2),
        "purity_karat": purity_karat,
        "base_24k_rate_per_gram": round(base_24k_rate, 2),
        "effective_rate_per_gram": round(effective_rate, 2),
        "pure_metal_cost": round(pure_metal_cost, 2),
        "making_charge_percent": round(making_charge_pct, 2),
        "making_charge_amount": round(making_amount, 2),
        "subtotal": round(subtotal, 2),
        "gst_percent": round(gst_pct, 2),
        "gst_amount": round(gst_amount, 2),
        "hallmarking_charge": round(hallmarking, 2),
        "total_investment_amount": round(total_acquisition_cost, 2),
        "total_acquisition_cost": round(total_acquisition_cost, 2),
        "tenure_years": round(tenure_years, 2),
        "expected_annual_appreciation_percent": round(appreciation_rate, 2),
        "projected_maturity_value": round(projected_future_metal_value, 2),
        "projected_gain": round(projected_gain, 2),
        "projected_return_percent": round(projected_return_pct, 2),
        "yearly_schedule": yearly_schedule,
        "schedule": yearly_schedule
    }
    return success(result)


