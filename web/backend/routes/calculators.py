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
        rate = float(data.get("annual_return_percent") or 0)
        years = int(data.get("duration_years") or 0)
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

        for m in range(1, 13):
            if depleted:
                break
            global_month = (y - 1) * 12 + m
            if global_month > total_months:
                break

            # 1. Growth phase
            interest = 0.0 if zero_rate else balance * r
            total_interest += interest
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
        "depleted_at_month": depleted_month,
        "depleted_at_year": ((depleted_month + 11) // 12) if depleted else 0,
        "yearly_breakdown": yearly_breakdown,
    }
    return success(result)

