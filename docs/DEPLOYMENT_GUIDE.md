# InvestSphere — Deployment & Hosting Guide
**Production Deployment, Configuration, and Operations Manual**  
**Document Version:** 1.0.0  
**Target Platform:** Render.com (Web Service / Blueprint)  

---

## 1. Hosting Architecture Overview

InvestSphere provides a unified web architecture designed for reliable public deployment across desktop, tablet, and mobile browsers:

```
[User Browser (Mobile / Desktop)]
              │
              ▼ HTTPS (TLS 1.3)
   ┌───────────────────────┐
   │    Render.com Edge    │ (SSL Termination, DDoS Protection)
   └──────────┬────────────┘
              │
              ▼
   ┌────────────────────────────────────────────────────────┐
   │             Gunicorn WSGI Application Server           │
   │                                                        │
   │  ┌──────────────────────┐    ┌──────────────────────┐  │
   │  │   Frontend Static    │    │    Flask REST API    │  │
   │  │ (HTML5 / CSS3 / JS)  │    │     (/api/*)         │  │
   │  └──────────────────────┘    └──────────┬───────────┘  │
   └─────────────────────────────────────────┼──────────────┘
                                             │
                                             ▼
                                  ┌────────────────────┐
                                  │   SQLite Database  │
                                  │ (WAL Mode, ACID)   │
                                  └────────────────────┘
```

- **Frontend:** Responsive Single-Page Application (SPA) built with clean HTML5, CSS3, and vanilla JavaScript. Served directly by the Flask application via static file routing, eliminating separate frontend hosting costs or CORS complications.
- **Backend:** Python 3.12+ Flask application running on production Gunicorn WSGI server with multi-threaded workers.
- **Database:** SQLite 3 with Write-Ahead Logging (`WAL` mode), foreign key enforcement, immutability triggers, and automatic schema initialization on startup.
- **Security:** HMAC-SHA256 signed session tokens, PBKDF2-SHA256 password hashing (100,000 rounds), parameterized SQL statements, and strict multi-tenant user data isolation.

---

## 2. Target Hosting Platform: Render.com

Render.com has been selected as the primary hosting provider because:
1. Native Python 3 runtime support with automated dependency caching (`pip`).
2. Free tier with automatic TLS/HTTPS certificates issued by Let's Encrypt.
3. Native support for `render.yaml` Infrastructure-as-Code blueprints.
4. Integrated health-check endpoint monitoring (`/api/health`).
5. Live streaming deployment and application logs.

---

## 3. Environment Variables Reference

Configure these environment variables in your hosting dashboard. **Never commit actual secrets or production passwords into version control.**

| Variable Name | Required | Default / Example | Purpose |
|---|---|---|---|
| `SECRET_KEY` | **Yes** | `(random 64-char hex string)` | Cryptographic secret used for HMAC token signatures and session integrity. Render generates this automatically if using `render.yaml`. |
| `FLASK_ENV` | **Yes** | `production` | Controls Flask execution mode (disables debug toolbar and unsafe stack traces). |
| `PORT` | **Yes** | `10000` (Render) / `5000` | Port on which Gunicorn binds the HTTP listener. |
| `DATABASE_URL` | Optional | `./investsphere_web.db` | Filesystem path for the SQLite database file. |
| `ALLOWED_ORIGINS` | Optional | `*` (or your domain URL) | Comma-delimited list of permitted CORS origins. |
| `DEMO_USER_EMAIL` | Optional | `demo@investsphere.demo` | Email address for seeding the non-privileged demo account. |
| `DEMO_USER_PASSWORD` | Optional | `DemoInvestor123!` | Password for seeding the non-privileged demo account. |

---

## 4. Build & Start Commands

- **Build Command:**
  ```bash
  pip install -r web/backend/requirements.txt
  ```
- **Start Command:**
  ```bash
  gunicorn --chdir web/backend wsgi:application --bind 0.0.0.0:$PORT --workers 2 --threads 4 --timeout 60
  ```
- **Health Check Endpoint:**
  ```http
  GET /api/health
  ```
  Expected Response: `{"status": "ok", "version": "1.0.0", "app": "InvestSphere Web", "timestamp": "..."}`

---

## 5. Step-by-Step Deployment Procedure

### Option A: Automated Deployment via Render Blueprint (Recommended)
1. Fork or push the `InvestSphere` repository to your GitHub or GitLab account.
2. Sign in to [Render.com](https://render.com).
3. In the Render Dashboard, click **New +** and select **Blueprint**.
4. Connect your InvestSphere Git repository. Render will automatically detect [`render.yaml`](file:///C:/InvestSphere/render.yaml).
5. Review the service configuration (`investsphere` web service, Oregon region, Python runtime).
6. Click **Apply**. Render will automatically build the service, install dependencies, initialize the database schema, and launch Gunicorn.

### Option B: Manual Web Service Setup on Render
1. In the Render Dashboard, click **New +** > **Web Service**.
2. Connect your Git repository.
3. Configure the following fields:
   - **Name:** `investsphere`
   - **Region:** Any (e.g. `Oregon (US West)` or `Frankfurt (EU Central)`)
   - **Branch:** `main`
   - **Root Directory:** Leave blank (root of repo)
   - **Runtime:** `Python 3`
   - **Build Command:** `pip install -r web/backend/requirements.txt`
   - **Start Command:** `gunicorn --chdir web/backend wsgi:application --bind 0.0.0.0:$PORT --workers 2 --threads 4 --timeout 60`
   - **Instance Type:** `Free`
4. Expand **Advanced** > **Environment Variables** and add:
   - `SECRET_KEY` = (Click "Generate" or paste a 64-character random string)
   - `FLASK_ENV` = `production`
   - `DATABASE_URL` = `/opt/render/project/src/web/backend/investsphere_web.db`
5. Set **Health Check Path** to `/api/health`.
6. Click **Create Web Service**.

---

## 6. Post-Deployment Verification Checklist

Once the build finishes and the service status displays **Live**, perform the following release verification:

1. **Health Check:**
   - Open `https://<your-subdomain>.onrender.com/api/health` in your browser. Confirm HTTP 200 with status `"ok"`.
2. **TLS / HTTPS Verification:**
   - Ensure browser shows a secure padlock with valid SSL/TLS certificate issued by Let's Encrypt.
3. **User Self-Registration:**
   - Open `https://<your-subdomain>.onrender.com/`.
   - Click "Create one" and register a test user account (`testuser@example.com`).
4. **Authentication:**
   - Sign in with the registered credentials. Confirm user name and email display on the sidebar.
5. **Add Investment:**
   - Click **+ Add Investment**. Add an equity stock (e.g., Symbol: `INFY`, Name: `Infosys Ltd`, Qty: `10`, Price: `1500`).
   - Confirm holding appears in both Dashboard and Portfolio pages.
6. **Record Transaction:**
   - Click **Record Transaction**. Record a `BUY` transaction.
   - Verify transaction appears in Transaction History and is reflected in the Portfolio Cost Basis.
7. **Calculators:**
   - Navigate to **SIP Calculator**, enter monthly amount ₹5,000, 12% p.a., 10 years. Verify wealth projection card and schedule table.
   - Navigate to **SWP Calculator**, enter ₹10,00,000 corpus, ₹8,000 withdrawal, 8% return. Verify solvency status and schedule table.
8. **Reports & CSV Export:**
   - Navigate to **Reports**, click **Export CSV** for Holdings and Transactions. Confirm `.csv` downloads properly with correct headers and figures.
9. **Maturity Tracker:**
   - Add a Fixed Deposit (e.g. SBI Term Deposit maturing in 30 days). Verify days-remaining countdown and badge coloring.
10. **Data Isolation Check:**
    - Sign out and register a second account (`user2@example.com`). Confirm that `user2` cannot view any investments, transactions, or stats belonging to the first user.

---

## 7. Demonstration Account Setup

To allow evaluators to test InvestSphere without entering manual transaction data, a dedicated seeder script is provided:

```bash
# Run seeder locally or via Render SSH Console:
python web/backend/seed_demo.py
```

### Demo Account Credentials
- **URL:** `https://<your-subdomain>.onrender.com/`
- **Email:** `demo@investsphere.demo`
- **Password:** `DemoInvestor123!`
- **Sample Portfolio:**
  - `RELIANCE` (Reliance Industries Ltd — 20 shares)
  - `TCS` (Tata Consultancy Services Ltd — 10 shares)
  - `NIFTYBEES` (Nippon India Nifty 50 ETF — 250 units)
  - `GOLDBEES` (Nippon India Gold ETF — 20 units)
  - `FD-SBI-2026` (SBI 1-Year Term Deposit — ₹1,50,000 @ 7.1%)

**Security Guarantee:** The demonstration account has standard non-administrator permissions and cannot view or tamper with any other user's portfolio data.

---

## 8. Free-Tier Operational Limitations & Best Practices

1. **Inactivity Sleep (Spin-Down):**  
   Render's free tier spins down web instances after 15 minutes of inactivity. When a new request arrives, Render boots the instance, which introduces a 30–50 second cold-start delay for the first request. Subsequent requests respond instantly (< 100ms).
2. **Disk Ephemerality:**  
   Render free tier instances use an ephemeral filesystem. If the container restarts or a new deployment occurs, the local SQLite database resets to initial state.
   - **For Long-Term Persistence:**
     - Option A: Attach a **Render Persistent Disk** ($1/month for 1 GB) mounted at `/data/investsphere.db`.
     - Option B: Connect to a free-tier managed cloud PostgreSQL database (e.g., Neon or Supabase) by setting `DATABASE_URL=postgresql://...`.
3. **Log Streaming:**  
   Live application logs can be monitored in real time under the **Logs** tab in the Render Dashboard.

