/**
 * InvestSphere Page Renderers
 * Generates UI views for each navigation section.
 */

function formatINR(num) {
  if (num === null || num === undefined || isNaN(num)) return '₹0.00';
  const val = Number(num);
  return '₹' + val.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 });
}

function formatNumber(num, decimals = 2) {
  if (num === null || num === undefined || isNaN(num)) return '0';
  return Number(num).toLocaleString('en-IN', { minimumFractionDigits: decimals, maximumFractionDigits: decimals });
}

function formatDate(dateStr) {
  if (!dateStr) return '—';
  try {
    const d = new Date(dateStr);
    return isNaN(d.getTime()) ? dateStr : d.toLocaleDateString('en-IN', { year: 'numeric', month: 'short', day: 'numeric' });
  } catch {
    return dateStr;
  }
}

function getCategoryBadge(cat) {
  const map = {
    'STOCK': { label: 'Stock', cls: 'badge-info' },
    'MUTUAL_FUND': { label: 'Mutual Fund', cls: 'badge-primary' },
    'GOLD': { label: 'Gold', cls: 'badge-warning' },
    'FIXED_DEPOSIT': { label: 'Fixed Deposit', cls: 'badge-success' },
    'BOND': { label: 'Bond', cls: 'badge-neutral' }
  };
  const b = map[cat] || { label: cat, cls: 'badge-neutral' };
  return `<span class="badge ${b.cls}">${b.label}</span>`;
}

function getTxTypeBadge(type) {
  const map = {
    'BUY': 'badge-primary',
    'SELL': 'badge-danger',
    'DIVIDEND': 'badge-success',
    'INTEREST': 'badge-success',
    'DEPOSIT': 'badge-info',
    'WITHDRAWAL': 'badge-warning',
    'FEE': 'badge-neutral'
  };
  return `<span class="badge ${map[type] || 'badge-neutral'}">${type}</span>`;
}

const Pages = {
  // ─── DASHBOARD ─────────────────────────────────────────────────────────────
  async renderDashboard(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/dashboard');
      const data = res.data || {};
      const sum = data.summary || {};
      const alloc = data.allocation || [];
      const txs = data.recent_transactions || [];
      const maturities = data.upcoming_maturities || [];

      const pnl = sum.total_gain_loss || 0;
      const pnlPct = sum.total_gain_loss_pct || 0;
      const pnlClass = pnl >= 0 ? 'text-success' : 'text-danger';
      const pnlCardClass = pnl >= 0 ? 'card-success' : 'card-danger';
      const pnlSign = pnl >= 0 ? '+' : '';

      let html = `
        <div class="page-header">
          <h2>Portfolio Overview</h2>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="App.navigate('transactions')">View All Transactions</button>
            <button class="btn btn-primary btn-sm" onclick="Modals.open('modal-add-investment')">+ Add Investment</button>
          </div>
        </div>

        <div class="summary-grid">
          <div class="summary-card card-primary">
            <div class="summary-label">Total Portfolio Value</div>
            <div class="summary-value">${formatINR(sum.total_current_value || 0)}</div>
            <div class="summary-sub">${sum.active_investments_count || 0} active investments</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">Total Invested Cost</div>
            <div class="summary-value">${formatINR(sum.total_invested || 0)}</div>
            <div class="summary-sub">Across all asset classes</div>
          </div>
          <div class="summary-card ${pnlCardClass}">
            <div class="summary-label">Total Returns (P&L)</div>
            <div class="summary-value ${pnlClass}">${pnlSign}${formatINR(pnl)}</div>
            <div class="summary-sub ${pnlClass}">${pnlSign}${formatNumber(pnlPct)}% overall return</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">Realized Returns</div>
            <div class="summary-value">${formatINR(sum.total_realized_gain || 0)}</div>
            <div class="summary-sub">Dividends, interest & sales</div>
          </div>
        </div>
      `;

      if ((sum.active_investments_count || 0) === 0) {
        html += `
          <div class="card">
            <div class="empty-state">
              <span class="empty-icon">📈</span>
              <h3>No Investments Yet</h3>
              <p>Start tracking your stocks, mutual funds, gold, fixed deposits, and bonds in one secure dashboard.</p>
              <button class="btn btn-primary mt-2" onclick="Modals.open('modal-add-investment')">Add Your First Investment</button>
            </div>
          </div>
        `;
        container.innerHTML = html;
        return;
      }

      html += `
        <div class="content-grid">
          <!-- Allocation Breakdown -->
          <div class="card">
            <div class="card-header">
              <div class="card-title">Asset Allocation</div>
              <button class="btn btn-outline btn-sm" onclick="App.navigate('analytics')">Detailed Analytics</button>
            </div>
            <div class="card-body">
              ${alloc.length === 0 ? '<p class="text-muted">No allocation data available.</p>' :
                alloc.map(a => `
                  <div style="margin-bottom: 0.9rem;">
                    <div style="display:flex; justify-content:space-between; font-size:0.85rem;">
                      <span class="font-bold">${a.category_name || a.category}</span>
                      <span>${formatINR(a.current_value)} <strong class="text-muted">(${formatNumber(a.percentage)}%)</strong></span>
                    </div>
                    <div class="alloc-bar">
                      <div class="alloc-bar-fill" style="width: ${Math.min(100, Math.max(2, a.percentage))}%; background: var(--primary);"></div>
                    </div>
                  </div>
                `).join('')
              }
            </div>
          </div>

          <!-- Upcoming Maturities -->
          <div class="card">
            <div class="card-header">
              <div class="card-title">Upcoming Maturities (Next 90 Days)</div>
              <button class="btn btn-outline btn-sm" onclick="App.navigate('maturity')">View All</button>
            </div>
            <div class="card-body">
              ${maturities.length === 0 ? '<p class="text-muted">No upcoming FD or bond maturities in the next 90 days.</p>' :
                `<div class="table-responsive">
                  <table>
                    <thead>
                      <tr><th>Instrument</th><th>Date</th><th>Amount</th><th>Days</th></tr>
                    </thead>
                    <tbody>
                      ${maturities.slice(0, 5).map(m => `
                        <tr>
                          <td><strong>${m.institution_name || m.instrument_name}</strong></td>
                          <td>${formatDate(m.maturity_date)}</td>
                          <td>${formatINR(m.maturity_amount || m.principal_amount)}</td>
                          <td><span class="badge ${m.days_remaining <= 15 ? 'badge-danger' : 'badge-warning'}">${m.days_remaining}d</span></td>
                        </tr>
                      `).join('')}
                    </tbody>
                  </table>
                </div>`
              }
            </div>
          </div>
        </div>

        <!-- Recent Transactions -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">Recent Transactions</div>
            <button class="btn btn-outline btn-sm" onclick="Modals.open('modal-add-transaction')">+ Record Transaction</button>
          </div>
          <div class="card-body">
            ${txs.length === 0 ? '<p class="text-muted">No transactions recorded yet.</p>' :
              `<div class="table-responsive">
                <table>
                  <thead>
                    <tr><th>Date</th><th>Type</th><th>Instrument</th><th>Quantity</th><th>Net Amount</th></tr>
                  </thead>
                  <tbody>
                    ${txs.slice(0, 5).map(t => `
                      <tr>
                        <td>${formatDate(t.transaction_date)}</td>
                        <td>${getTxTypeBadge(t.transaction_type)}</td>
                        <td><strong>${t.instrument_name || t.symbol || '—'}</strong></td>
                        <td>${t.quantity ? formatNumber(t.quantity, 4) : '—'}</td>
                        <td class="font-bold">${formatINR(t.net_amount)}</td>
                      </tr>
                    `).join('')}
                  </tbody>
                </table>
              </div>`
            }
          </div>
        </div>
      `;

      container.innerHTML = html;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load dashboard: ${err.message}</div>`;
    }
  },

  // ─── PORTFOLIO ─────────────────────────────────────────────────────────────
  async renderPortfolio(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/investments');
      const list = res.data || [];

      // Calculate Real-time Portfolio Totals
      let totalInvested = 0;
      let totalCurrentValue = 0;
      let totalDayGain = 0;

      list.forEach(inv => {
        const cost = inv.total_invested || inv.principal_amount || 0;
        const val = (inv.current_value !== null && inv.current_value !== undefined) ? inv.current_value : cost;
        totalInvested += cost;
        totalCurrentValue += val;
        if (inv.day_change && inv.quantity) {
          totalDayGain += (inv.day_change * inv.quantity);
        }
      });

      const overallPnl = totalCurrentValue - totalInvested;
      const overallPnlPct = totalInvested > 0 ? (overallPnl / totalInvested) * 100 : 0;
      const dayGainPct = totalCurrentValue > 0 ? (totalDayGain / totalCurrentValue) * 100 : 0;

      let html = `
        <div class="page-header">
          <div>
            <h2>Investment Portfolio</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Live holdings with real-time market prices, average purchase cost, and overall profit & loss.
            </p>
          </div>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="App.refreshCurrentPage()">↻ Refresh Live</button>
            <button class="btn btn-outline btn-sm" onclick="Modals.open('modal-add-transaction')">+ Record Transaction</button>
            <button class="btn btn-primary btn-sm" onclick="Modals.open('modal-add-investment')">+ Add Investment</button>
          </div>
        </div>

        <!-- Overall Portfolio Performance Summary Cards -->
        <div class="summary-grid" style="grid-template-columns: repeat(4, 1fr); gap: 1rem; margin-bottom: 1.5rem;">
          <div class="summary-card card-primary">
            <div class="summary-label">PORTFOLIO LIVE VALUE</div>
            <div class="summary-value font-bold" style="font-size:1.45rem;">${formatINR(totalCurrentValue)}</div>
            <div class="summary-sub text-muted">${list.length} Total Holdings</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">TOTAL INVESTED (COST)</div>
            <div class="summary-value" style="font-size:1.45rem;">${formatINR(totalInvested)}</div>
            <div class="summary-sub text-muted">Net acquisition capital</div>
          </div>
          <div class="summary-card ${overallPnl >= 0 ? 'card-success' : 'card-danger'}">
            <div class="summary-label">TOTAL PROFIT / LOSS</div>
            <div class="summary-value ${overallPnl >= 0 ? 'text-success' : 'text-danger'} font-bold" style="font-size:1.45rem;">
              ${overallPnl >= 0 ? '+' : ''}${formatINR(overallPnl)}
            </div>
            <div class="summary-sub ${overallPnl >= 0 ? 'text-success' : 'text-danger'} font-bold">
              ${overallPnl >= 0 ? '▲' : '▼'} ${overallPnl >= 0 ? '+' : ''}${formatNumber(overallPnlPct)}% Total Return
            </div>
          </div>
          <div class="summary-card">
            <div class="summary-label">TODAY'S RETURN (DAY GAIN)</div>
            <div class="summary-value ${totalDayGain >= 0 ? 'text-success' : 'text-danger'}" style="font-size:1.45rem;">
              ${totalDayGain >= 0 ? '+' : ''}${formatINR(totalDayGain)}
            </div>
            <div class="summary-sub text-muted">
              ${totalDayGain >= 0 ? '+' : ''}${formatNumber(dayGainPct)}% vs Prev Close
            </div>
          </div>
        </div>

        <div class="filter-bar">
          <select id="portfolio-filter-cat" onchange="Pages.filterPortfolioTable()">
            <option value="">All Categories</option>
            <option value="STOCK">Stocks</option>
            <option value="MUTUAL_FUND">Mutual Funds</option>
            <option value="GOLD">Gold</option>
            <option value="FIXED_DEPOSIT">Fixed Deposits</option>
            <option value="BOND">Bonds</option>
          </select>
          <select id="portfolio-filter-status" onchange="Pages.filterPortfolioTable()">
            <option value="ACTIVE">Active Only</option>
            <option value="">All Statuses</option>
            <option value="CLOSED">Closed Only</option>
          </select>
          <input type="text" id="portfolio-search" placeholder="Search by name or symbol…" oninput="Pages.filterPortfolioTable()"/>
        </div>

        <div class="card">
          <div class="card-body">
            ${list.length === 0 ? `
              <div class="empty-state">
                <span class="empty-icon">💼</span>
                <h3>No Investments Found</h3>
                <p>Click the button below to add your first stock, mutual fund, FD, bond, or gold holding.</p>
                <button class="btn btn-primary mt-2" onclick="Modals.open('modal-add-investment')">+ Add Investment</button>
              </div>
            ` : `
              <div class="table-responsive">
                <table id="portfolio-table">
                  <thead>
                    <tr>
                      <th>Category</th>
                      <th>Symbol</th>
                      <th>Name</th>
                      <th>Quantity</th>
                      <th>Avg Buy Price (₹)</th>
                      <th>Live Market Price (₹)</th>
                      <th>Cost Basis (₹)</th>
                      <th>Current Value (₹)</th>
                      <th>Gain / Loss</th>
                      <th>Status</th>
                      <th>Actions</th>
                    </tr>
                  </thead>
                  <tbody>
                    ${list.map(inv => {
                      const cost = inv.total_invested || inv.principal_amount || 0;
                      const val = (inv.current_value !== null && inv.current_value !== undefined) ? inv.current_value : cost;
                      const gl = inv.unrealized_gain_loss !== undefined ? inv.unrealized_gain_loss : (val - cost);
                      const glPct = cost > 0 ? (gl / cost) * 100 : 0;
                      const glClass = gl >= 0 ? 'text-success' : 'text-danger';
                      const glSign = gl >= 0 ? '+' : '';
                      const qty = inv.quantity || 0;
                      const avgBuyPrice = qty > 0 ? (cost / qty) : 0;
                      const livePrice = inv.latest_price || (qty > 0 ? (val / qty) : avgBuyPrice);
                      const dayChange = inv.day_change || 0;
                      const dayChangePct = inv.day_change_pct || 0;

                      return `
                        <tr data-cat="${inv.type_code}" data-status="${inv.status}" data-search="${((inv.name || '') + ' ' + (inv.symbol || '')).toLowerCase()}">
                          <td>${getCategoryBadge(inv.type_code)}</td>
                          <td><strong>${inv.symbol || '—'}</strong></td>
                          <td>${inv.name || '—'}</td>
                          <td>${inv.quantity ? formatNumber(inv.quantity, 4) : '—'}</td>
                          <td>${avgBuyPrice > 0 ? formatINR(avgBuyPrice) : '—'}</td>
                          <td class="font-bold">
                            ${livePrice > 0 ? formatINR(livePrice) : '—'}
                            ${dayChange ? `<br><small class="${dayChange >= 0 ? 'text-success' : 'text-danger'}" style="font-weight:normal;">${dayChange >= 0 ? '+' : ''}${formatINR(dayChange)} (${dayChange >= 0 ? '+' : ''}${formatNumber(dayChangePct)}%)</small>` : ''}
                          </td>
                          <td>${formatINR(cost)}</td>
                          <td class="font-bold">${formatINR(val)}</td>
                          <td class="${glClass} font-bold">
                            ${glSign}${formatINR(gl)} <br><small>(${glSign}${formatNumber(glPct)}%)</small>
                          </td>
                          <td><span class="badge ${inv.status === 'ACTIVE' ? 'badge-success' : 'badge-neutral'}">${inv.status}</span></td>
                          <td>
                            <div style="display:flex; gap:0.3rem;">
                              <button class="btn btn-outline btn-sm" onclick="Pages.recordTxForInv(${inv.investment_id})" title="Record Transaction">+ Tx</button>
                              <button class="btn btn-outline btn-sm text-danger" onclick="Pages.deleteInvestment(${inv.investment_id}, '${(inv.name || '').replace(/'/g, "\\'")}')" title="Delete">✕</button>
                            </div>
                          </td>
                        </tr>
                      `;
                    }).join('')}
                  </tbody>
                </table>
              </div>
            `}
          </div>
        </div>
      `;

      container.innerHTML = html;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load portfolio: ${err.message}</div>`;
    }
  },

  filterPortfolioTable() {
    const cat = document.getElementById('portfolio-filter-cat')?.value || '';
    const status = document.getElementById('portfolio-filter-status')?.value || '';
    const search = (document.getElementById('portfolio-search')?.value || '').toLowerCase().trim();

    const rows = document.querySelectorAll('#portfolio-table tbody tr');
    rows.forEach(r => {
      const rCat = r.getAttribute('data-cat') || '';
      const rStatus = r.getAttribute('data-status') || '';
      const rSearch = r.getAttribute('data-search') || '';

      const matchCat = !cat || rCat === cat;
      const matchStatus = !status || rStatus === status;
      const matchSearch = !search || rSearch.includes(search);

      r.style.display = (matchCat && matchStatus && matchSearch) ? '' : 'none';
    });
  },

  async recordTxForInv(invId) {
    Modals.open('modal-add-transaction');
    setTimeout(() => {
      const sel = document.getElementById('tx-investment');
      if (sel) sel.value = String(invId);
    }, 200);
  },

  async deleteInvestment(invId, name) {
    if (!confirm(`Are you sure you want to delete investment "${name}" and all related transactions?`)) {
      return;
    }
    try {
      const res = await Api.delete(`/investments/${invId}`);
      if (res.success || res.status === 'ok') {
        showToast(`Investment deleted: ${name}`, 'success');
        App.refreshCurrentPage();
      } else {
        throw new Error(res.error || 'Failed to delete');
      }
    } catch (err) {
      showToast(err.message, 'error');
    }
  },

  // ─── TRANSACTIONS ──────────────────────────────────────────────────────────
  async renderTransactions(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/transactions');
      const list = res.data || [];

      let html = `
        <div class="page-header">
          <h2>Transaction History</h2>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="Api.download('/reports/transactions?format=csv', 'transactions.csv')">Export CSV</button>
            <button class="btn btn-primary btn-sm" onclick="Modals.open('modal-add-transaction')">+ Record Transaction</button>
          </div>
        </div>

        <div class="card">
          <div class="card-body">
            ${list.length === 0 ? `
              <div class="empty-state">
                <span class="empty-icon">📋</span>
                <h3>No Transactions Yet</h3>
                <p>Record your purchases, sales, dividends, or interest receipts.</p>
                <button class="btn btn-primary mt-2" onclick="Modals.open('modal-add-transaction')">Record First Transaction</button>
              </div>
            ` : `
              <div class="table-responsive">
                <table>
                  <thead>
                    <tr>
                      <th>Date</th>
                      <th>Type</th>
                      <th>Instrument</th>
                      <th>Quantity</th>
                      <th>Gross (₹)</th>
                      <th>Fees & Taxes</th>
                      <th>Net Amount (₹)</th>
                      <th>Notes</th>
                      <th>Actions</th>
                    </tr>
                  </thead>
                  <tbody>
                    ${list.map(t => `
                      <tr>
                        <td>${formatDate(t.transaction_date)}</td>
                        <td>${getTxTypeBadge(t.transaction_type)}</td>
                        <td><strong>${t.instrument_name || t.symbol || '—'}</strong></td>
                        <td>${t.quantity ? formatNumber(t.quantity, 4) : '—'}</td>
                        <td>${formatINR(t.gross_amount)}</td>
                        <td class="text-muted">${formatINR((t.fees || 0) + (t.taxes || 0))}</td>
                        <td class="font-bold">${formatINR(t.net_amount)}</td>
                        <td class="text-muted"><small>${t.notes || '—'}</small></td>
                        <td>
                          <button class="btn btn-outline btn-sm text-danger" onclick="Pages.deleteTransaction(${t.transaction_id})" title="Delete">✕</button>
                        </td>
                      </tr>
                    `).join('')}
                  </tbody>
                </table>
              </div>
            `}
          </div>
        </div>
      `;

      container.innerHTML = html;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load transactions: ${err.message}</div>`;
    }
  },

  async deleteTransaction(txId) {
    if (!confirm('Are you sure you want to delete this transaction?')) return;
    try {
      const res = await Api.delete(`/transactions/${txId}`);
      if (res.success || res.status === 'ok') {
        showToast('Transaction deleted', 'success');
        App.refreshCurrentPage();
      } else {
        throw new Error(res.error || 'Failed to delete');
      }
    } catch (err) {
      showToast(err.message, 'error');
    }
  },

  // ─── STOCKS & LIVE MARKET WATCH (PHASE 2) ─────────────────────────────────
  async renderStocks(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const [overviewRes, quotesRes] = await Promise.all([
        Api.get('/market/overview').catch(() => ({ data: {} })),
        Api.get('/market/stocks?limit=600').catch(() => Api.get('/market/quotes').catch(() => ({ data: [] })))
      ]);

      const overview = overviewRes.data || {};
      const indices = overview.indices || [];
      const allQuotes = Array.isArray(quotesRes.data)
        ? quotesRes.data
        : (quotesRes.data?.stocks || quotesRes.data?.quotes || []);
      const marketStatus = overview.market_status || 'CLOSED (After Hours)';
      const statusClass = marketStatus.includes('OPEN') ? 'badge-success' : 'badge-neutral';

      let html = `
        <div class="page-header">
          <div>
            <h2>Stocks & Live Market</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Live and benchmark Indian equity feeds powered by Yahoo Finance (NSE & BSE).
            </p>
          </div>
          <div class="topbar-actions" style="display:flex; align-items:center; gap:0.75rem;">
            <span class="badge ${statusClass}">Market: ${marketStatus}</span>
            <button class="btn btn-outline btn-sm" onclick="App.refreshCurrentPage()">↻ Refresh</button>
          </div>
        </div>

        <!-- Benchmark Index Cards -->
        <div class="summary-grid" style="grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); margin-bottom: 1.5rem;">
          ${indices.map(idx => {
            const chg = idx.change || 0;
            const chgPct = idx.change_percent || 0;
            const isUp = chg >= 0;
            const sign = isUp ? '+' : '';
            const colorClass = isUp ? 'text-success' : 'text-danger';
            return `
              <div class="index-card ${isUp ? 'index-up' : 'index-down'}">
                <div style="display:flex; justify-content:space-between; align-items:center;">
                  <span class="summary-label" style="font-weight:700;">${idx.symbol}</span>
                  <span class="badge badge-neutral" style="font-size:0.7rem;">${idx.exchange || 'NSE'}</span>
                </div>
                <div class="summary-value" style="font-size:1.35rem; margin:0.3rem 0;">${formatINR(idx.price)}</div>
                <div class="summary-sub ${colorClass} font-bold">${sign}${formatINR(chg)} (${sign}${formatNumber(chgPct)}%)</div>
              </div>
            `;
          }).join('')}
        </div>

        <!-- Filter & Search Bar -->
        <div class="card">
          <div class="card-header" style="flex-wrap:wrap; gap:0.75rem;">
            <div class="pill-bar" style="margin-bottom:0;" id="stocks-filter-pills">
              <button class="pill-btn active" onclick="Pages.filterStocksTab('ALL')">All Equities (${allQuotes.length})</button>
              <button class="pill-btn" onclick="Pages.filterStocksTab('GAINERS')">Top Gainers 📈</button>
              <button class="pill-btn" onclick="Pages.filterStocksTab('LOSERS')">Top Losers 📉</button>
              <button class="pill-btn" onclick="Pages.loadWatchlistTab()">★ My Watchlist</button>
            </div>
            <div style="flex:1; max-width:280px;">
              <input type="text" id="stock-search-input" placeholder="🔍 Search stock by symbol or name…" onkeyup="Pages.searchStocksTable(this.value)"/>
            </div>
          </div>
          <div class="card-body">
            <div class="table-responsive">
              <table id="stocks-table">
                <thead>
                  <tr>
                    <th>Symbol</th>
                    <th>Company Name</th>
                    <th>Exchange</th>
                    <th>Price (₹)</th>
                    <th>Day Change</th>
                    <th>52W High / Low</th>
                    <th>Data Source</th>
                    <th>Actions</th>
                  </tr>
                </thead>
                <tbody id="stocks-table-body">
                  ${allQuotes.map(q => Pages.renderStockTableRow(q)).join('')}
                </tbody>
              </table>
            </div>
          </div>
        </div>

        <!-- Stock Chart / Detail Modal Container -->
        <div class="modal-backdrop hidden" id="modal-stock-chart">
          <div class="modal" style="max-width: 650px;">
            <div class="modal-header">
              <h3 id="modal-stock-title">Stock Analysis</h3>
              <button class="modal-close" onclick="document.getElementById('modal-stock-chart').classList.add('hidden')">✕</button>
            </div>
            <div class="modal-body" id="modal-stock-body">
              <div class="spinner"></div>
            </div>
          </div>
        </div>
      `;

      container.innerHTML = html;
      window._investsphere_all_stocks = allQuotes;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load stock data: ${err.message}</div>`;
    }
  },

  renderStockTableRow(q) {
    const chg = q.change || 0;
    const chgPct = q.change_percent || 0;
    const isUp = chg >= 0;
    const sign = isUp ? '+' : '';
    const colorClass = isUp ? 'text-success' : 'text-danger';
    const high = q.fifty_two_week_high ? formatINR(q.fifty_two_week_high) : '—';
    const low = q.fifty_two_week_low ? formatINR(q.fifty_two_week_low) : '—';

    return `
      <tr data-symbol="${q.symbol}" data-name="${(q.name || '').toLowerCase()}" data-change="${chg}">
        <td><strong>${q.symbol}</strong></td>
        <td>${q.name || q.symbol}</td>
        <td><span class="badge badge-neutral">${q.exchange || 'NSE'}</span></td>
        <td class="font-bold">${formatINR(q.price)}</td>
        <td class="${colorClass} font-bold">${sign}${formatINR(chg)} (${sign}${formatNumber(chgPct)}%)</td>
        <td style="font-size:0.85rem;">H: ${high} <br/><span class="text-muted">L: ${low}</span></td>
        <td style="font-size:0.8rem;"><span class="badge badge-info">${q.data_source || 'Yahoo Finance'}</span></td>
        <td>
          <div style="display:flex; gap:0.4rem;">
            <button class="btn btn-primary btn-sm" title="Buy and Add to Portfolio" onclick="Modals.openAddInvestmentForStock('${q.symbol}', '${(q.name || q.symbol).replace(/'/g, "\\'")}', ${q.price || 0})">+ Buy</button>
            <button class="btn btn-outline btn-sm" onclick="Pages.openStockChart('${q.symbol}')">📈 Chart</button>
            <button class="btn btn-outline btn-sm" title="Add to Watchlist" onclick="Pages.addToWatchlist('${q.symbol}', '${(q.name || q.symbol).replace(/'/g, "\\'")}')">★</button>
          </div>
        </td>
      </tr>
    `;
  },

  renderMarket(container) {
    return this.renderStocks(container);
  },

  searchStocksTable(query) {
    const q = query.toLowerCase().trim();
    const rows = document.querySelectorAll('#stocks-table-body tr');
    rows.forEach(r => {
      const sym = r.getAttribute('data-symbol')?.toLowerCase() || '';
      const name = r.getAttribute('data-name') || '';
      if (!q || sym.includes(q) || name.includes(q)) {
        r.style.display = '';
      } else {
        r.style.display = 'none';
      }
    });
  },

  filterStocksTab(filter) {
    document.querySelectorAll('#stocks-filter-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    const rows = Array.from(document.querySelectorAll('#stocks-table-body tr'));
    rows.forEach(r => {
      const chg = parseFloat(r.getAttribute('data-change') || 0);
      if (filter === 'GAINERS') {
        r.style.display = chg >= 0 ? '' : 'none';
      } else if (filter === 'LOSERS') {
        r.style.display = chg < 0 ? '' : 'none';
      } else {
        r.style.display = '';
      }
    });
  },

  async loadWatchlistTab() {
    document.querySelectorAll('#stocks-filter-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    const tbody = document.getElementById('stocks-table-body');
    if (!tbody) return;
    tbody.innerHTML = '<tr><td colspan="8" class="text-center py-3"><div class="spinner"></div></td></tr>';

    try {
      const res = await Api.get('/market/watchlist');
      const items = res.data || [];
      if (items.length === 0) {
        tbody.innerHTML = '<tr><td colspan="8" class="text-center py-4 text-muted">Your watchlist is empty. Click ★ on any stock to add it here.</td></tr>';
        return;
      }
      tbody.innerHTML = items.map(item => {
        const q = item.quote || { symbol: item.symbol, name: item.instrument_name, price: 0, change: 0, change_percent: 0 };
        return Pages.renderStockTableRow(q);
      }).join('');
    } catch (err) {
      tbody.innerHTML = `<tr><td colspan="8" class="text-center text-danger">${err.message}</td></tr>`;
    }
  },

  async addToWatchlist(symbol, name) {
    try {
      await Api.post('/market/watchlist', { symbol, name });
      if (typeof showToast === 'function') {
        showToast(`${symbol} added to your watchlist.`, 'success');
      }
    } catch (err) {
      if (typeof showToast === 'function') {
        showToast(err.message, 'error');
      }
    }
  },

  async openStockChart(symbol) {
    const modal = document.getElementById('modal-stock-chart');
    const title = document.getElementById('modal-stock-title');
    const body = document.getElementById('modal-stock-body');
    if (!modal || !body) return;

    modal.classList.remove('hidden');
    title.textContent = `${symbol} — Historical Performance & Analysis`;
    body.innerHTML = '<div class="spinner"></div>';

    try {
      const [detailRes, histRes] = await Promise.all([
        Api.get(`/market/detail/${symbol}`),
        Api.get(`/market/history/${symbol}?period=1mo`)
      ]);

      const d = detailRes.data || {};
      const hist = histRes.data?.history || [];
      const chg = d.change || 0;
      const chgPct = d.change_percent || 0;
      const isUp = chg >= 0;
      const sign = isUp ? '+' : '';
      const colorClass = isUp ? 'text-success' : 'text-danger';

      // SVG Sparkline Chart
      let chartSvg = '<p class="text-muted text-center py-3">No historical chart data available.</p>';
      if (hist.length > 1) {
        const prices = hist.map(p => p.close);
        const minP = Math.min(...prices);
        const maxP = Math.max(...prices);
        const range = maxP - minP || 1;
        const width = 580;
        const height = 180;
        const padding = 20;

        const points = prices.map((p, idx) => {
          const x = padding + (idx / (prices.length - 1)) * (width - 2 * padding);
          const y = height - padding - ((p - minP) / range) * (height - 2 * padding);
          return `${x.toFixed(1)},${y.toFixed(1)}`;
        }).join(' ');

        const strokeColor = isUp ? '#16a34a' : '#dc2626';
        chartSvg = `
          <svg viewBox="0 0 ${width} ${height}" style="width:100%; height:180px; background:#f8fafc; border-radius:8px; border:1px solid #e2e8f0;">
            <polyline fill="none" stroke="${strokeColor}" stroke-width="2.5" points="${points}" stroke-linecap="round" stroke-linejoin="round"/>
            <text x="${padding}" y="${padding - 5}" font-size="11" fill="#64748b">₹${formatNumber(maxP)} (High)</text>
            <text x="${padding}" y="${height - 5}" font-size="11" fill="#64748b">₹${formatNumber(minP)} (Low)</text>
            <text x="${width - padding - 80}" y="${height - 5}" font-size="11" fill="#64748b">${hist[0]?.date} → ${hist[hist.length-1]?.date}</text>
          </svg>
        `;
      }

      body.innerHTML = `
        <div style="display:flex; justify-content:space-between; align-items:baseline; margin-bottom:1rem;">
          <div>
            <div style="font-size:1.4rem; font-weight:700;">${d.name || symbol}</div>
            <div class="text-muted" style="font-size:0.85rem;">Exchange: ${d.exchange || 'NSE'} • Sector: ${d.sector || 'Equities'}</div>
          </div>
          <div style="text-align:right;">
            <div style="font-size:1.5rem; font-weight:700;">${formatINR(d.price)}</div>
            <div class="${colorClass} font-bold">${sign}${formatINR(chg)} (${sign}${formatNumber(chgPct)}%)</div>
          </div>
        </div>

        <div style="margin-bottom:1rem;">
          <div style="font-weight:600; font-size:0.85rem; margin-bottom:0.4rem; color:var(--text-muted);">30-DAY PRICE TREND</div>
          ${chartSvg}
        </div>

        <div class="summary-grid" style="grid-template-columns: repeat(3, 1fr); gap:0.75rem; margin-bottom:1rem;">
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">DAY OPEN</div>
            <div class="summary-value" style="font-size:1rem;">${formatINR(d.open)}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">DAY HIGH</div>
            <div class="summary-value text-success" style="font-size:1rem;">${formatINR(d.high)}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">DAY LOW</div>
            <div class="summary-value text-danger" style="font-size:1rem;">${formatINR(d.low)}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">52-WEEK HIGH</div>
            <div class="summary-value" style="font-size:1rem;">${d.fifty_two_week_high ? formatINR(d.fifty_two_week_high) : '—'}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">52-WEEK LOW</div>
            <div class="summary-value" style="font-size:1rem;">${d.fifty_two_week_low ? formatINR(d.fifty_two_week_low) : '—'}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label" style="font-size:0.75rem;">VOLUME</div>
            <div class="summary-value" style="font-size:1rem;">${d.volume ? Number(d.volume).toLocaleString('en-IN') : '—'}</div>
          </div>
        </div>

        <div style="font-size:0.8rem; color:var(--text-muted); display:flex; justify-content:space-between;">
          <span>Data Provider: <strong>${d.data_source || 'Yahoo Finance'}</strong></span>
          <span>Last Updated: ${formatDate(d.last_updated)}</span>
        </div>
      `;
    } catch (err) {
      body.innerHTML = `<div class="alert alert-error">Failed to load stock details: ${err.message}</div>`;
    }
  },

  // ─── IPO CENTER (PHASE 3) ──────────────────────────────────────────────────
  async renderIPO(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const [sumRes, ipoRes] = await Promise.all([
        Api.get('/ipo/summary').catch(() => ({ data: {} })),
        Api.get('/ipo').catch(() => ({ data: { ipos: [] } }))
      ]);

      const sum = sumRes.data || {};
      const ipoPayload = ipoRes.data || {};
      const ipos = ipoPayload.ipos || [];
      const dataSource = ipoPayload.data_source || 'ipoalerts.in / NSE & BSE Official';
      const lastUpdated = ipoPayload.last_updated ? new Date(ipoPayload.last_updated).toLocaleTimeString('en-IN', { hour: '2-digit', minute: '2-digit', second: '2-digit' }) : 'Live';
      const staleWarning = ipoPayload.stale_warning;

      let html = `
        <div class="page-header">
          <div>
            <h2>IPO Dashboard & Offerings</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Track Indian Mainboard & SME Initial Public Offerings with live ipoalerts data and official NSE/BSE exchange verification.
            </p>
          </div>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="Pages.refreshIPOData()">↻ Refresh Live</button>
          </div>
        </div>

        <!-- Data Source & Verification Status Banner -->
        <div style="display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:0.5rem; background:#f8fafc; border:1px solid #e2e8f0; border-radius:8px; padding:0.6rem 0.9rem; margin-bottom:1.25rem; font-size:0.82rem;">
          <div style="display:flex; align-items:center; flex-wrap:wrap; gap:0.5rem; color:#475569;">
            <span>📡 Data Provider: <strong>${dataSource}</strong></span>
            <span>•</span>
            <span>Verification: <strong>NSE / BSE / SEBI</strong></span>
            <span>•</span>
            <span>Updated: <strong>${lastUpdated}</strong></span>
          </div>
          <div style="display:flex; align-items:center; gap:0.5rem;">
            ${staleWarning ? `<span class="badge badge-warning" style="font-size:0.75rem;" title="${staleWarning}">⚠️ ${staleWarning}</span>` : '<span class="badge badge-success" style="font-size:0.75rem;">● Live & Verified</span>'}
          </div>
        </div>

        <!-- Summary Metric Cards -->
        <div class="summary-grid" style="grid-template-columns: repeat(4, 1fr); margin-bottom: 1.5rem;">
          <div class="summary-card">
            <div class="summary-label">Upcoming IPOs</div>
            <div class="summary-value text-primary">${sum.upcoming || 0}</div>
            <div class="summary-sub text-muted">Announced schedules</div>
          </div>
          <div class="summary-card card-success">
            <div class="summary-label">Open / Ongoing</div>
            <div class="summary-value text-success">${sum.open || 0}</div>
            <div class="summary-sub text-success font-bold">Bidding currently active</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">Recently Listed</div>
            <div class="summary-value font-bold">${sum.listed || 0}</div>
            <div class="summary-sub text-muted">Debut gainers & momentum</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">SME Offerings</div>
            <div class="summary-value text-info">${sum.sme || 0}</div>
            <div class="summary-sub text-muted">NSE Emerge & BSE SME</div>
          </div>
        </div>

        <!-- Filter Pills, Sort & Search -->
        <div class="card">
          <div class="card-header" style="flex-wrap:wrap; gap:0.75rem;">
            <div class="pill-bar" style="margin-bottom:0;" id="ipo-filter-pills">
              <button class="pill-btn active" data-tab="ALL" onclick="Pages.filterIPOTab('ALL')">All Offerings</button>
              <button class="pill-btn" data-tab="OPEN" onclick="Pages.filterIPOTab('OPEN')">Open Now 🟢</button>
              <button class="pill-btn" data-tab="UPCOMING" onclick="Pages.filterIPOTab('UPCOMING')">Upcoming 📅</button>
              <button class="pill-btn" data-tab="LISTED" onclick="Pages.filterIPOTab('LISTED')">Recently Listed 📈</button>
              <button class="pill-btn" data-tab="SME" onclick="Pages.filterIPOTab('SME')">SME IPOs</button>
              <button class="pill-btn" data-tab="MAINBOARD" onclick="Pages.filterIPOTab('MAINBOARD')">Mainboard</button>
            </div>
            <div style="display:flex; gap:0.5rem; align-items:center; flex:1; max-width:440px;">
              <select id="ipo-sort-select" class="form-control" style="flex:1; padding:0.4rem 0.6rem; font-size:0.85rem;" onchange="Pages.sortIPOCards(this.value)">
                <option value="date-desc">Sort: Latest Date 📅</option>
                <option value="date-asc">Sort: Oldest Date</option>
                <option value="size-desc">Sort: Issue Size (High to Low)</option>
                <option value="size-asc">Sort: Issue Size (Low to High)</option>
                <option value="gain-desc">Sort: Listing Gain (High to Low)</option>
                <option value="name-asc">Sort: Company Name (A-Z)</option>
              </select>
              <input type="text" id="ipo-search-input" placeholder="🔍 Search company or symbol…" onkeyup="Pages.searchIPOCards(this.value)" style="flex:1.2;"/>
            </div>
          </div>

          <div class="card-body">
            <div class="ipo-grid" id="ipo-cards-container">
              ${ipos.map(ipo => Pages.renderIPOCard(ipo)).join('')}
            </div>
          </div>
        </div>

        <!-- IPO Detail Modal -->
        <div class="modal-backdrop hidden" id="modal-ipo-detail">
          <div class="modal" style="max-width: 650px;">
            <div class="modal-header">
              <h3 id="modal-ipo-title">IPO Details</h3>
              <button class="modal-close" onclick="document.getElementById('modal-ipo-detail').classList.add('hidden')">✕</button>
            </div>
            <div class="modal-body" id="modal-ipo-body">
              <div class="spinner"></div>
            </div>
          </div>
        </div>
      `;

      container.innerHTML = html;
      window._investsphere_all_ipos = ipos;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load IPO data: ${err.message}</div>`;
    }
  },

  async refreshIPOData() {
    const container = document.getElementById('ipo-cards-container');
    if (container) {
      container.innerHTML = '<div class="spinner"></div>';
    }
    try {
      const [sumRes, ipoRes] = await Promise.all([
        Api.get('/ipo/summary').catch(() => ({ data: {} })),
        Api.get('/ipo?refresh=true').catch(() => ({ data: { ipos: [] } }))
      ]);
      const mainContainer = document.getElementById('page-content') || document.querySelector('.main-content');
      if (mainContainer) {
        Pages.renderIPO(mainContainer);
      }
    } catch (err) {
      console.error('Refresh IPO error:', err);
    }
  },

  sortIPOCards(sortVal) {
    if (!window._investsphere_all_ipos) return;
    const [field, order] = sortVal.split('-');
    const sorted = [...window._investsphere_all_ipos].sort((a, b) => {
      let valA, valB;
      if (field === 'date') {
        valA = a.opening_date || a.listing_date || '';
        valB = b.opening_date || b.listing_date || '';
        return order === 'asc' ? valA.localeCompare(valB) : valB.localeCompare(valA);
      } else if (field === 'size') {
        valA = a.issue_size_cr || 0;
        valB = b.issue_size_cr || 0;
        return order === 'asc' ? valA - valB : valB - valA;
      } else if (field === 'gain') {
        valA = a.listing_gain_pct || -9999;
        valB = b.listing_gain_pct || -9999;
        return order === 'asc' ? valA - valB : valB - valA;
      } else if (field === 'name') {
        valA = (a.company_name || '').toLowerCase();
        valB = (b.company_name || '').toLowerCase();
        return order === 'asc' ? valA.localeCompare(valB) : valB.localeCompare(valA);
      }
      return 0;
    });
    const container = document.getElementById('ipo-cards-container');
    if (container) {
      container.innerHTML = sorted.map(ipo => Pages.renderIPOCard(ipo)).join('');
      const activeTab = document.querySelector('#ipo-filter-pills .pill-btn.active')?.getAttribute('data-tab') || 'ALL';
      Pages.filterIPOTab(activeTab);
    }
  },

  renderIPOCard(ipo) {
    let statusBadge = '<span class="badge badge-neutral">Announced</span>';
    if (ipo.status === 'OPEN') {
      statusBadge = '<span class="badge badge-success">Open for Bidding</span>';
    } else if (ipo.status === 'LISTED') {
      statusBadge = '<span class="badge badge-primary">Listed on Exchange</span>';
    } else if (ipo.status === 'CLOSED') {
      statusBadge = '<span class="badge badge-warning">Closed / Allotment</span>';
    } else if (ipo.status === 'UPCOMING') {
      statusBadge = '<span class="badge badge-info">Upcoming</span>';
    }

    const typeBadge = ipo.ipo_type === 'SME' ?
      '<span class="badge badge-warning">SME</span>' :
      '<span class="badge badge-info">Mainboard</span>';

    // Listing Gain or Subscription Highlight
    let highlight = '';
    if (ipo.status === 'LISTED' && ipo.listing_gain_pct !== null && ipo.listing_gain_pct !== undefined) {
      const isGain = ipo.listing_gain_pct >= 0;
      const sign = isGain ? '+' : '';
      const cls = isGain ? 'text-success' : 'text-danger';
      const listPriceStr = ipo.listing_price ? ` (${formatINR(ipo.listing_price)})` : '';
      highlight = `<div style="font-size:0.85rem; font-weight:700;" class="${cls}">Debut: ${sign}${formatNumber(ipo.listing_gain_pct)}%${listPriceStr}</div>`;
    } else if (ipo.subscription_status) {
      highlight = `<div style="font-size:0.85rem; font-weight:600; color:var(--primary);">${ipo.subscription_status}</div>`;
    }

    const minInvStr = (ipo.min_investment && ipo.min_investment > 0) ? formatINR(ipo.min_investment) : '<span class="text-muted">Not Announced</span>';
    const lotSizeStr = (ipo.lot_size && ipo.lot_size > 0) ? `${ipo.lot_size} Shares` : '<span class="text-muted">Not Announced</span>';
    const issueSizeStr = (ipo.issue_size_cr && ipo.issue_size_cr > 0) ? `₹${formatNumber(ipo.issue_size_cr)} Cr` : '<span class="text-muted">Data Unavailable</span>';
    const priceBandStr = ipo.price_band || 'Not Announced';
    const sourceLabel = ipo.source_name || (ipo.is_live ? 'ipoalerts.in' : 'NSE/BSE Official');

    return `
      <div class="ipo-card" data-status="${ipo.status}" data-type="${ipo.ipo_type}" data-name="${(ipo.company_name || '').toLowerCase()}" data-symbol="${(ipo.symbol || '').toLowerCase()}">
        <div>
          <div class="ipo-header">
            <div>
              <div class="ipo-title">${ipo.company_name}</div>
              <div class="ipo-symbol">${ipo.symbol} • ${ipo.exchange || 'NSE/BSE'}</div>
            </div>
            <div style="display:flex; flex-direction:column; align-items:flex-end; gap:0.3rem;">
              ${typeBadge}
              ${statusBadge}
            </div>
          </div>

          <div class="ipo-metrics-grid">
            <div>
              <div class="ipo-metric-label">PRICE BAND</div>
              <div class="ipo-metric-val">${priceBandStr}</div>
            </div>
            <div>
              <div class="ipo-metric-label">MIN INVESTMENT</div>
              <div class="ipo-metric-val">${minInvStr}</div>
            </div>
            <div>
              <div class="ipo-metric-label">LOT SIZE</div>
              <div class="ipo-metric-val">${lotSizeStr}</div>
            </div>
            <div>
              <div class="ipo-metric-label">ISSUE SIZE</div>
              <div class="ipo-metric-val">${issueSizeStr}</div>
            </div>
          </div>

          <div style="display:flex; justify-content:space-between; align-items:center; margin:0.4rem 0;">
            <div style="font-size:0.8rem; color:var(--text-muted);">
              Dates: <strong>${formatDate(ipo.opening_date)}</strong> – <strong>${formatDate(ipo.closing_date)}</strong>
            </div>
            ${highlight}
          </div>
        </div>

        <div style="margin-top:1rem; padding-top:0.75rem; border-top:1px solid #f1f5f9; display:flex; justify-content:space-between; align-items:center;">
          <span style="font-size:0.75rem; color:var(--text-muted);" title="Verified from ${sourceLabel}">Listing: ${formatDate(ipo.listing_date)}</span>
          <button class="btn btn-outline btn-sm" onclick="Pages.openIPODetail('${ipo.id}')">View Details & Stats ↗</button>
        </div>
      </div>
    `;
  },

  filterIPOTab(tab) {
    document.querySelectorAll('#ipo-filter-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    const targetBtn = document.querySelector(`#ipo-filter-pills .pill-btn[data-tab="${tab}"]`);
    if (targetBtn) {
      targetBtn.classList.add('active');
    } else if (event?.target?.classList?.contains('pill-btn')) {
      event.target.classList.add('active');
    }

    const cards = document.querySelectorAll('#ipo-cards-container .ipo-card');
    cards.forEach(card => {
      const status = card.getAttribute('data-status');
      const type = card.getAttribute('data-type');

      if (tab === 'ALL') {
        card.style.display = '';
      } else if (tab === 'MAINBOARD' || tab === 'SME') {
        card.style.display = type === tab ? '' : 'none';
      } else {
        card.style.display = status === tab ? '' : 'none';
      }
    });
  },

  searchIPOCards(query) {
    const q = query.toLowerCase().trim();
    const cards = document.querySelectorAll('#ipo-cards-container .ipo-card');
    cards.forEach(card => {
      const name = card.getAttribute('data-name') || '';
      const sym = card.getAttribute('data-symbol') || '';
      if (!q || name.includes(q) || sym.includes(q)) {
        card.style.display = '';
      } else {
        card.style.display = 'none';
      }
    });
  },

  async openIPODetail(identifier) {
    const modal = document.getElementById('modal-ipo-detail');
    const title = document.getElementById('modal-ipo-title');
    const body = document.getElementById('modal-ipo-body');
    if (!modal || !body) return;

    modal.classList.remove('hidden');
    body.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.get(`/ipo/${identifier}`);
      const ipo = res.data || {};
      title.textContent = `${ipo.company_name} (${ipo.symbol})`;

      const stats = ipo.subscription_stats || {};
      const qib = stats.qib ? `${stats.qib}x` : '—';
      const nii = stats.nii ? `${stats.nii}x` : '—';
      const ret = stats.retail ? `${stats.retail}x` : '—';
      const tot = stats.total ? `${stats.total}x` : (ipo.subscription_status || '—');

      const freshIssueStr = (ipo.fresh_issue_cr !== null && ipo.fresh_issue_cr !== undefined) ? `₹${formatNumber(ipo.fresh_issue_cr)} Cr` : 'Data Unavailable';
      const ofsStr = (ipo.ofs_cr !== null && ipo.ofs_cr !== undefined) ? `₹${formatNumber(ipo.ofs_cr)} Cr` : 'Data Unavailable';
      const issueSizeStr = (ipo.issue_size_cr) ? `₹${formatNumber(ipo.issue_size_cr)} Cr` : 'Data Unavailable';

      const retailQuota = (ipo.retail_quota_pct !== null && ipo.retail_quota_pct !== undefined) ? `${ipo.retail_quota_pct}%` : '—';
      const qibQuota = (ipo.qib_quota_pct !== null && ipo.qib_quota_pct !== undefined) ? `${ipo.qib_quota_pct}%` : '—';
      const niiQuota = (ipo.nii_quota_pct !== null && ipo.nii_quota_pct !== undefined) ? `${ipo.nii_quota_pct}%` : '—';

      const sourceLink = ipo.source_url ? `<a href="${ipo.source_url}" target="_blank" rel="noopener noreferrer" style="color:var(--primary); text-decoration:underline;">${ipo.source_name || 'Official Verification Source'} ↗</a>` : (ipo.source_name || 'NSE/BSE India');

      body.innerHTML = `
        <div style="margin-bottom:1rem;">
          <p style="font-size:0.92rem; color:var(--text); line-height:1.5;">${ipo.description || 'Verified Indian Initial Public Offering details.'}</p>
        </div>

        <div style="font-weight:600; font-size:0.85rem; color:var(--text-muted); margin-bottom:0.4rem;">OFFERING SPECIFICATIONS</div>
        <div class="summary-grid" style="grid-template-columns: repeat(2, 1fr); gap:0.6rem; margin-bottom:1.25rem;">
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label">PRICE BAND</div>
            <div class="summary-value" style="font-size:1.1rem;">${ipo.price_band || 'Not Announced'}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label">TOTAL ISSUE SIZE</div>
            <div class="summary-value" style="font-size:1.1rem;">${issueSizeStr}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label">FRESH ISSUE</div>
            <div class="summary-value" style="font-size:1rem;">${freshIssueStr}</div>
          </div>
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label">OFFER FOR SALE (OFS)</div>
            <div class="summary-value" style="font-size:1rem;">${ofsStr}</div>
          </div>
        </div>

        <div style="font-weight:600; font-size:0.85rem; color:var(--text-muted); margin-bottom:0.4rem;">SUBSCRIPTION STATISTICS</div>
        <div class="summary-grid" style="grid-template-columns: repeat(4, 1fr); gap:0.5rem; margin-bottom:1.25rem;">
          <div class="summary-card" style="padding:0.6rem; text-align:center;">
            <div class="summary-label" style="font-size:0.7rem;">QIB (${qibQuota})</div>
            <div class="font-bold text-primary" style="font-size:1.05rem;">${qib}</div>
          </div>
          <div class="summary-card" style="padding:0.6rem; text-align:center;">
            <div class="summary-label" style="font-size:0.7rem;">NII (${niiQuota})</div>
            <div class="font-bold text-info" style="font-size:1.05rem;">${nii}</div>
          </div>
          <div class="summary-card" style="padding:0.6rem; text-align:center;">
            <div class="summary-label" style="font-size:0.7rem;">RETAIL (${retailQuota})</div>
            <div class="font-bold text-success" style="font-size:1.05rem;">${ret}</div>
          </div>
          <div class="summary-card" style="padding:0.6rem; text-align:center;">
            <div class="summary-label" style="font-size:0.7rem;">OVERALL</div>
            <div class="font-bold" style="font-size:1.05rem;">${tot}</div>
          </div>
        </div>

        <div style="background:#f8fafc; padding:0.85rem; border-radius:8px; font-size:0.82rem; color:var(--text-muted); line-height:1.6;">
          <div><strong>Registrar:</strong> ${ipo.registrar || 'Data Unavailable'}</div>
          <div><strong>Lead Managers:</strong> ${Array.isArray(ipo.lead_managers) ? ipo.lead_managers.join(', ') : (ipo.lead_managers || 'Data Unavailable')}</div>
          <div><strong>Important Schedule:</strong> Open: ${formatDate(ipo.opening_date)} • Close: ${formatDate(ipo.closing_date)} • Allotment: ${formatDate(ipo.allotment_date)} • Listing: ${formatDate(ipo.listing_date)}</div>
          <div style="margin-top:0.35rem;"><strong>Source Verification:</strong> ${sourceLink}</div>
        </div>
      `;
    } catch (err) {
      body.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  // ─── MARKET NEWS (PHASE 4) ─────────────────────────────────────────────────
  async renderNews(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const [catsRes, newsRes] = await Promise.all([
        Api.get('/news/categories').catch(() => ({ data: [] })),
        Api.get('/news?limit=12').catch(() => ({ data: { articles: [] } }))
      ]);

      const categories = catsRes.data || [];
      const articles = newsRes.data?.articles || [];

      let html = `
        <div class="page-header">
          <div>
            <h2>Market Insights & Financial News</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Curated financial headlines, corporate earnings, IPO filings, and RBI macroeconomic updates.
            </p>
          </div>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="App.refreshCurrentPage()">↻ Refresh News</button>
          </div>
        </div>

        <!-- 11 Category Pills Filter Bar -->
        <div class="card" style="margin-bottom:1.5rem;">
          <div class="card-body" style="padding:1rem;">
            <div style="display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:0.75rem;">
              <div class="pill-bar" style="margin-bottom:0;" id="news-category-pills">
                ${categories.map(c => `
                  <button class="pill-btn ${c.code === 'ALL' ? 'active' : ''}" onclick="Pages.filterNewsCategory('${c.code}')">
                    ${c.icon || '📰'} ${c.name} (${c.count})
                  </button>
                `).join('')}
              </div>
              <div style="min-width:240px;">
                <input type="text" id="news-search-input" placeholder="🔍 Search news headlines…" onkeyup="Pages.searchNewsCards(this.value)"/>
              </div>
            </div>
          </div>
        </div>

        <!-- Articles Grid -->
        <div class="news-grid" id="news-articles-container">
          ${articles.map(a => Pages.renderNewsCard(a)).join('')}
        </div>
      `;

      container.innerHTML = html;
      window._investsphere_all_news = articles;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load market news: ${err.message}</div>`;
    }
  },

  renderNewsCard(a) {
    const fallbackImg = "https://images.unsplash.com/photo-1611974789855-9c2a0a7236a3?w=600&auto=format&fit=crop&q=80";
    return `
      <div class="news-card" data-category="${a.category}" data-headline="${(a.headline || '').toLowerCase()}">
        <img class="news-thumb" src="${a.thumbnail || fallbackImg}" alt="News thumbnail" onerror="this.src='${fallbackImg}'"/>
        <div class="news-body">
          <div class="news-meta">
            <span><strong>${a.source || 'Market Desk'}</strong></span>
            <span>${formatDate(a.published_at)}</span>
          </div>
          <a href="${a.url}" target="_blank" rel="noopener noreferrer" class="news-title">${a.headline}</a>
          <p class="news-summary">${a.summary}</p>
          <div style="display:flex; justify-content:space-between; align-items:center; margin-top:auto;">
            <span class="badge badge-info" style="font-size:0.72rem;">${a.category_name || a.category}</span>
            <a href="${a.url}" target="_blank" rel="noopener noreferrer" class="news-link">Read Full Story ↗</a>
          </div>
        </div>
      </div>
    `;
  },

  renderMarketNews(container) {
    return this.renderNews(container);
  },

  async filterNewsCategory(catCode) {
    document.querySelectorAll('#news-category-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    const container = document.getElementById('news-articles-container');
    if (!container) return;
    container.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.get(`/news?category=${catCode}&limit=16`);
      const articles = res.data?.articles || [];
      if (articles.length === 0) {
        container.innerHTML = '<div class="card p-4 text-center text-muted" style="grid-column: 1 / -1;">No articles currently found for this category.</div>';
        return;
      }
      container.innerHTML = articles.map(a => Pages.renderNewsCard(a)).join('');
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  searchNewsCards(query) {
    const q = query.toLowerCase().trim();
    const cards = document.querySelectorAll('#news-articles-container .news-card');
    cards.forEach(card => {
      const headline = card.getAttribute('data-headline') || '';
      if (!q || headline.includes(q)) {
        card.style.display = '';
      } else {
        card.style.display = 'none';
      }
    });
  },

  // ─── GOLD & SILVER TRACKING IN INDIA (PHASE 5) ────────────────────────────
  async renderPreciousMetals(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const [citiesRes, ratesRes] = await Promise.all([
        Api.get('/precious-metals/cities').catch(() => ({ data: [] })),
        Api.get('/precious-metals/rates?city=pune').catch(() => ({ data: {} }))
      ]);

      const cities = citiesRes.data || [];
      const rates = ratesRes.data || {};

      let html = `
        <div class="page-header">
          <div>
            <h2>Gold & Silver Rates (OroPocket Live Bullion Feed)</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Live digital bullion spot rates fetched from OroPocket API with Buy/Sell quotes, 24h change, and retail jewellery billing calculator.
            </p>
          </div>
          <div style="display:flex; align-items:center; gap:0.75rem;">
            <label for="metals-city-select" style="font-size:0.85rem; font-weight:600;">Location Context:</label>
            <select id="metals-city-select" style="width:200px;" onchange="Pages.updateMetalRatesCity(this.value)">
              ${cities.map(c => `
                <option value="${c.key}" ${c.key === 'pune' ? 'selected' : ''}>${c.name} (${c.hub})</option>
              `).join('')}
            </select>
            <button class="btn btn-outline btn-sm" onclick="Pages.refreshMetalRates()" title="Refresh live rates from OroPocket">↻ Live Refresh</button>
          </div>
        </div>

        <div class="alert alert-info" style="border-left: 4px solid #3b82f6;">
          📢 <strong>Indicative Rates Notice:</strong> Rates shown reflect national spot benchmark prices in <strong>INR per gram</strong> fetched from OroPocket's public bullion API.
          <strong>These rates are indicative digital bullion prices and NOT exact local Maharashtra retail jeweller OTC prices</strong>, which vary by city sarafa association daily rates, local transport cess, custom making charges (8%–15%), and hallmarking fees.
          <div style="font-size:0.82rem; margin-top:5px; color:var(--text-muted);">
            Location Hub: <strong>${rates.city || 'Pune'}</strong> (${rates.market_hub || 'Laxmi Road'}) • 
            Last Updated: <strong>${rates.last_updated ? new Date(rates.last_updated).toLocaleString('en-IN') : 'Just now'}</strong> • 
            Data Source: <strong>${rates.data_source || 'OroPocket Public API'}</strong> • Auto-cached (5m refresh)
          </div>
        </div>

        <!-- 4 Metal Rate Cards -->
        <div class="metal-cards-grid" id="metal-rate-cards">
          ${Pages.renderPreciousMetalsCards(rates)}
        </div>

        <!-- Two Column Layout: Calculator & Historical Trend -->
        <div class="content-grid" style="grid-template-columns: 1fr 1fr; gap:1.5rem; margin-top:1.5rem;">
          <!-- Retail Jewellery Bill Calculator -->
          <div class="card">
            <div class="card-header">
              <div class="card-title">💍 Retail Jewellery Cost Estimator (Live Bullion Base)</div>
            </div>
            <div class="card-body">
              <div class="form-row">
                <div class="form-group">
                  <label>Purity Karat</label>
                  <select id="calc-gold-purity" onchange="Pages.calculateJewelleryBill()">
                    <option value="22" selected>22K Gold (91.6% Hallmark Jewellery)</option>
                    <option value="18">18K Gold (75.0% Diamond/Stone)</option>
                    <option value="24">24K Bullion (99.9% Bar/Coin)</option>
                  </select>
                </div>
                <div class="form-group">
                  <label>Weight (Grams)</label>
                  <input type="number" id="calc-gold-weight" value="10" min="0.5" max="1000" step="0.5" oninput="Pages.calculateJewelleryBill()"/>
                </div>
              </div>

              <div class="form-group">
                <label>Making Charges (<span id="calc-making-val">12%</span>)</label>
                <input type="range" id="calc-making-range" min="8" max="20" value="12" step="1" oninput="document.getElementById('calc-making-val').textContent = this.value + '%'; Pages.calculateJewelleryBill()"/>
                <small class="field-hint">Indian retail jewelers typically charge 8% to 15% making charges based on craftsmanship.</small>
              </div>

              <div id="jewellery-bill-result" style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:8px; padding:1rem; margin-top:1rem;">
                <!-- Result populated by JS -->
              </div>
            </div>
          </div>

          <!-- Historical Price Trend SVG -->
          <div class="card">
            <div class="card-header" style="justify-content:space-between;">
              <div class="card-title">📈 Price Trend & History</div>
              <div style="display:flex; gap:0.4rem;">
                <button class="pill-btn active" onclick="Pages.loadMetalHistoricalTrend('gold', '30d')">Gold 30D</button>
                <button class="pill-btn" onclick="Pages.loadMetalHistoricalTrend('gold', '1y')">Gold 1Y</button>
                <button class="pill-btn" onclick="Pages.loadMetalHistoricalTrend('silver', '30d')">Silver 30D</button>
              </div>
            </div>
            <div class="card-body" id="metal-trend-container">
              <div class="spinner"></div>
            </div>
          </div>
        </div>
      `;

      container.innerHTML = html;
      this.calculateJewelleryBill();
      this.loadMetalHistoricalTrend('gold', '30d');
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load precious metals data: ${err.message}</div>`;
    }
  },

  renderPreciousMetalsCards(rates) {
    const g24 = rates.gold_24k || {};
    const g22 = rates.gold_22k || {};
    const g18 = rates.gold_18k || {};
    const sil = rates.silver || {};

    const chg24 = g24.change_pct_buy || g24.daily_change_pct || 0;
    const chg22 = g22.change_pct_buy || g22.daily_change_pct || 0;
    const chg18 = g18.change_pct_buy || g18.daily_change_pct || 0;
    const chgSil = sil.change_pct_buy || sil.daily_change_pct || 0;

    const sign24 = chg24 >= 0 ? '+' : '';
    const sign22 = chg22 >= 0 ? '+' : '';
    const sign18 = chg18 >= 0 ? '+' : '';
    const signSil = chgSil >= 0 ? '+' : '';

    return `
      <!-- 24K Gold -->
      <div class="metal-card gold-card-24k">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.5rem;">
          <strong style="color:#d97706; font-size:1.05rem;">24K Pure Gold</strong>
          <span class="badge badge-warning">99.9% Purity</span>
        </div>
        <div style="font-size:1.6rem; font-weight:700; color:var(--text);">${formatINR(g24.buy_price_per_gram || g24.per_gram)} <span style="font-size:0.8rem; font-weight:normal; color:var(--text-muted);">/ gram</span></div>
        <div style="font-size:0.88rem; color:var(--text-muted); margin-bottom:0.3rem;">
          Sell Rate: <strong>${formatINR(g24.sell_price_per_gram || (g24.per_gram * 0.965))}</strong> / g
        </div>
        <div class="${chg24 >= 0 ? 'text-success' : 'text-danger'} font-bold" style="font-size:0.85rem; margin-bottom:0.75rem;">
          ${sign24}${formatNumber(chg24)}% 24h change
        </div>
        <div class="metal-rate-row"><span>Per 10 Grams:</span> <strong>${formatINR(g24.per_10_gram)}</strong></div>
        <div class="metal-rate-row"><span>Per 1 Kilogram:</span> <strong>${formatINR(g24.per_kg)}</strong></div>
        <div class="metal-rate-row"><span>Indicative Spread:</span> <strong>${formatINR(g24.spread_per_gram || 489.11)}/g</strong></div>
        <div style="font-size:0.72rem; color:var(--text-muted); margin-top:0.4rem; text-align:right;">Indicative OroPocket Spot Rate</div>
      </div>

      <!-- 22K Gold -->
      <div class="metal-card gold-card-22k">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.5rem;">
          <strong style="color:#b45309; font-size:1.05rem;">22K Jewellery Gold</strong>
          <span class="badge badge-warning">91.6% Hallmark</span>
        </div>
        <div style="font-size:1.6rem; font-weight:700; color:var(--text);">${formatINR(g22.buy_price_per_gram || g22.per_gram)} <span style="font-size:0.8rem; font-weight:normal; color:var(--text-muted);">/ gram</span></div>
        <div style="font-size:0.88rem; color:var(--text-muted); margin-bottom:0.3rem;">
          Sell Rate: <strong>${formatINR(g22.sell_price_per_gram || (g22.per_gram * 0.965))}</strong> / g
        </div>
        <div class="${chg22 >= 0 ? 'text-success' : 'text-danger'} font-bold" style="font-size:0.85rem; margin-bottom:0.75rem;">
          ${sign22}${formatNumber(chg22)}% 24h change
        </div>
        <div class="metal-rate-row"><span>Per 8g (1 Pavan):</span> <strong>${formatINR((g22.per_gram || 0) * 8)}</strong></div>
        <div class="metal-rate-row"><span>Per 10 Grams:</span> <strong>${formatINR(g22.per_10_gram)}</strong></div>
        <div class="metal-rate-row"><span>Purity Ratio:</span> <strong>91.67% of 24K</strong></div>
        <div style="font-size:0.72rem; color:var(--text-muted); margin-top:0.4rem; text-align:right;">Indicative Jewellery Benchmark</div>
      </div>

      <!-- 18K Gold -->
      <div class="metal-card gold-card-18k">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.5rem;">
          <strong style="color:#854d0e; font-size:1.05rem;">18K Diamond Gold</strong>
          <span class="badge badge-neutral">75.0% Purity</span>
        </div>
        <div style="font-size:1.6rem; font-weight:700; color:var(--text);">${formatINR(g18.buy_price_per_gram || g18.per_gram)} <span style="font-size:0.8rem; font-weight:normal; color:var(--text-muted);">/ gram</span></div>
        <div style="font-size:0.88rem; color:var(--text-muted); margin-bottom:0.3rem;">
          Sell Rate: <strong>${formatINR(g18.sell_price_per_gram || (g18.per_gram * 0.965))}</strong> / g
        </div>
        <div class="${chg18 >= 0 ? 'text-success' : 'text-danger'} font-bold" style="font-size:0.85rem; margin-bottom:0.75rem;">
          ${sign18}${formatNumber(chg18)}% 24h change
        </div>
        <div class="metal-rate-row"><span>Per 10 Grams:</span> <strong>${formatINR(g18.per_10_gram)}</strong></div>
        <div class="metal-rate-row"><span>Common Use:</span> <strong>Diamond Studded</strong></div>
        <div class="metal-rate-row"><span>Purity Ratio:</span> <strong>75.00% of 24K</strong></div>
        <div style="font-size:0.72rem; color:var(--text-muted); margin-top:0.4rem; text-align:right;">Indicative Hallmark Benchmark</div>
      </div>

      <!-- Fine Silver -->
      <div class="metal-card silver-card">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.5rem;">
          <strong style="color:#475569; font-size:1.05rem;">Fine Silver</strong>
          <span class="badge badge-info">99.9% Pure</span>
        </div>
        <div style="font-size:1.6rem; font-weight:700; color:var(--text);">${formatINR(sil.buy_price_per_gram || sil.per_gram)} <span style="font-size:0.8rem; font-weight:normal; color:var(--text-muted);">/ gram</span></div>
        <div style="font-size:0.88rem; color:var(--text-muted); margin-bottom:0.3rem;">
          Sell Rate: <strong>${formatINR(sil.sell_price_per_gram || (sil.per_gram * 0.944))}</strong> / g
        </div>
        <div class="${chgSil >= 0 ? 'text-success' : 'text-danger'} font-bold" style="font-size:0.85rem; margin-bottom:0.75rem;">
          ${signSil}${formatNumber(chgSil)}% 24h change
        </div>
        <div class="metal-rate-row"><span>Per 10 Grams:</span> <strong>${formatINR(sil.per_10_gram)}</strong></div>
        <div class="metal-rate-row"><span>Per 1 Kilogram:</span> <strong>${formatINR(sil.per_kg)}</strong></div>
        <div class="metal-rate-row"><span>Indicative Spread:</span> <strong>${formatINR(sil.spread_per_gram || 10.39)}/g</strong></div>
        <div style="font-size:0.72rem; color:var(--text-muted); margin-top:0.4rem; text-align:right;">Indicative OroPocket Spot Rate</div>
      </div>
    `;
  },

  async refreshMetalRates() {
    const cityKey = document.getElementById('metals-city-select')?.value || 'pune';
    const cardsContainer = document.getElementById('metal-rate-cards');
    if (cardsContainer) cardsContainer.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get(`/precious-metals/rates?city=${cityKey}&refresh=1`);
      const rates = res.data || {};
      if (cardsContainer) cardsContainer.innerHTML = Pages.renderPreciousMetalsCards(rates);
      Pages.calculateJewelleryBill();
      if (typeof showToast === 'function') {
        showToast('Precious metals rates refreshed from OroPocket.', 'success');
      }
    } catch (err) {
      if (cardsContainer) cardsContainer.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async updateMetalRatesCity(cityKey) {
    const cardsContainer = document.getElementById('metal-rate-cards');
    const cityLabel = document.getElementById('metals-current-city');
    if (!cardsContainer) return;
    cardsContainer.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.get(`/precious-metals/rates?city=${cityKey}`);
      const rates = res.data || {};
      if (cityLabel) cityLabel.textContent = rates.city || cityKey;
      cardsContainer.innerHTML = Pages.renderPreciousMetalsCards(rates);
      Pages.calculateJewelleryBill();
    } catch (err) {
      cardsContainer.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async calculateJewelleryBill() {
    const purityEl = document.getElementById('calc-gold-purity');
    const weightEl = document.getElementById('calc-gold-weight');
    const rangeEl = document.getElementById('calc-making-range');
    const resultBox = document.getElementById('jewellery-bill-result');
    if (!purityEl || !weightEl || !resultBox) return;

    const purity_karat = parseInt(purityEl.value, 10);
    const weight_grams = parseFloat(weightEl.value || 0);
    const making_charge_pct = parseFloat(rangeEl ? rangeEl.value : 12);

    if (weight_grams <= 0) {
      resultBox.innerHTML = '<p class="text-muted">Enter valid weight in grams.</p>';
      return;
    }

    try {
      const res = await Api.post('/precious-metals/calculate', {
        purity_karat,
        weight_grams,
        making_charge_pct
      });
      const d = res.data || {};

      resultBox.innerHTML = `
        <div style="display:flex; justify-content:space-between; margin-bottom:0.4rem; font-size:0.88rem;">
          <span>Pure Metal Cost (${d.weight_grams}g @ ${formatINR(d.gold_rate_per_gram)}/g):</span>
          <strong>${formatINR(d.base_gold_cost)}</strong>
        </div>
        <div style="display:flex; justify-content:space-between; margin-bottom:0.4rem; font-size:0.88rem;">
          <span>Making Charges (${d.making_charge_pct}%):</span>
          <strong>+${formatINR(d.making_charge_amount)}</strong>
        </div>
        <div style="display:flex; justify-content:space-between; margin-bottom:0.4rem; font-size:0.88rem;">
          <span>GST (3% on Subtotal):</span>
          <strong>+${formatINR(d.gst_amount)}</strong>
        </div>
        <div style="display:flex; justify-content:space-between; margin-bottom:0.6rem; font-size:0.88rem;">
          <span>BIS Hallmarking Fee:</span>
          <strong>+${formatINR(d.hallmarking_charge)}</strong>
        </div>
        <div style="display:flex; justify-content:space-between; padding-top:0.6rem; border-top:1.5px solid #cbd5e1; font-size:1.15rem; font-weight:700; color:var(--primary);">
          <span>Estimated Total Payable:</span>
          <span>${formatINR(d.total_estimated_payable)}</span>
        </div>
      `;
    } catch (err) {
      resultBox.innerHTML = `<span class="text-danger">${err.message}</span>`;
    }
  },

  async loadMetalHistoricalTrend(metal, period) {
    const container = document.getElementById('metal-trend-container');
    if (!container) return;
    container.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.get(`/precious-metals/history?metal=${metal}&period=${period}`);
      const data = res.data || {};
      const series = data.series || [];

      if (series.length < 2) {
        container.innerHTML = '<p class="text-muted text-center py-4">No trend series available.</p>';
        return;
      }

      const prices = series.map(p => p.price);
      const minP = Math.min(...prices);
      const maxP = Math.max(...prices);
      const range = maxP - minP || 1;
      const width = 500;
      const height = 180;
      const pad = 20;

      const points = prices.map((p, idx) => {
        const x = pad + (idx / (prices.length - 1)) * (width - 2 * pad);
        const y = height - pad - ((p - minP) / range) * (height - 2 * pad);
        return `${x.toFixed(1)},${y.toFixed(1)}`;
      }).join(' ');

      const isUp = (data.total_change || 0) >= 0;
      const strokeColor = isUp ? '#16a34a' : '#dc2626';
      const sign = isUp ? '+' : '';

      container.innerHTML = `
        <div style="display:flex; justify-content:space-between; align-items:baseline; margin-bottom:0.6rem;">
          <div>
            <strong>${data.metal}</strong>
            <span class="text-muted" style="font-size:0.8rem;">(${data.period})</span>
          </div>
          <div class="${isUp ? 'text-success' : 'text-danger'} font-bold">
            ${sign}${formatINR(data.total_change)} (${sign}${formatNumber(data.total_change_pct)}%)
          </div>
        </div>

        <svg viewBox="0 0 ${width} ${height}" style="width:100%; height:180px; background:#f8fafc; border-radius:8px; border:1px solid #e2e8f0;">
          <polyline fill="none" stroke="${strokeColor}" stroke-width="2.5" points="${points}" stroke-linecap="round" stroke-linejoin="round"/>
          <text x="${pad}" y="${pad - 4}" font-size="11" fill="#64748b">₹${formatNumber(maxP)}</text>
          <text x="${pad}" y="${height - 4}" font-size="11" fill="#64748b">₹${formatNumber(minP)}</text>
          <text x="${width - pad - 75}" y="${height - 4}" font-size="11" fill="#64748b">${series[0]?.date} → ${series[series.length-1]?.date}</text>
        </svg>
      `;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  // ─── SIP & MUTUAL FUNDS DISCOVERY (PHASE 6) ────────────────────────────────
  async renderSIPAndMutualFunds(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      let html = `
        <div class="page-header">
          <div>
            <h2>SIP & Mutual Funds Wealth Management</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Systematic Investment Planning, Step-Up simulation, goal-based recommendation wizard, and Indian mutual fund discovery.
            </p>
          </div>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="App.refreshCurrentPage()">↻ Refresh</button>
          </div>
        </div>

        <!-- Sub Tabs -->
        <div class="sub-tabs" id="sip-main-tabs">
          <button class="sub-tab-btn active" onclick="Pages.switchSIPTab('calc')">📊 SIP Calculator & Step-Up</button>
          <button class="sub-tab-btn" onclick="Pages.switchSIPTab('wizard')">🎯 Goal Recommendation Wizard</button>
          <button class="sub-tab-btn" onclick="Pages.switchSIPTab('explorer')">🔍 Mutual Fund Explorer</button>
          <button class="sub-tab-btn" onclick="Pages.switchSIPTab('comparison')">⚖️ Fund Comparison</button>
          <button class="sub-tab-btn" onclick="Pages.switchSIPTab('portfolio')">💼 My SIP Portfolio</button>
        </div>

        <!-- TAB 1: SIP CALCULATOR -->
        <div id="tab-sip-calc" class="sip-tab-pane">
          <div class="calc-layout">
            <div class="card">
              <div class="card-header">
                <div class="card-title">Investment Parameters</div>
              </div>
              <div class="card-body">
                <div class="form-group">
                  <label>Monthly Investment Amount (₹) *</label>
                  <input type="number" id="sip-calc-amount" value="5000" min="500" step="500" oninput="Pages.runSIPCalc()"/>
                </div>
                <div class="form-group">
                  <label>Expected Annual Return Rate (%) *</label>
                  <input type="number" id="sip-calc-rate" value="12" min="1" max="40" step="0.5" oninput="Pages.runSIPCalc()"/>
                  <small class="field-hint">Historic Indian equity index benchmark: 12% to 15% CAGR.</small>
                </div>
                <div class="form-group">
                  <label>Investment Tenure (Years) *</label>
                  <input type="number" id="sip-calc-years" value="10" min="1" max="40" step="1" oninput="Pages.runSIPCalc()"/>
                </div>
                <div class="form-group">
                  <label>Annual Step-Up (% increase per year) <small class="text-muted">— Optional</small></label>
                  <input type="number" id="sip-calc-stepup" value="5" min="0" max="30" step="1" oninput="Pages.runSIPCalc()"/>
                  <small class="field-hint">e.g. 5% step-up increases monthly investment by 5% every 12 months as your income rises.</small>
                </div>
                <div style="display:flex; gap:0.5rem; margin-top:1rem;">
                  <button class="btn btn-primary" style="flex:1;" onclick="Pages.runSIPCalc()">Calculate Returns</button>
                  <button class="btn btn-outline" onclick="Pages.openSaveSIPModal()">+ Save to My SIPs</button>
                </div>
              </div>
            </div>

            <div class="card">
              <div class="card-header">
                <div class="card-title">Projected Wealth Accumulation</div>
              </div>
              <div class="card-body" id="sip-calc-result-box">
                <div class="spinner"></div>
              </div>
            </div>
          </div>

          <div class="card mt-3" id="sip-breakdown-card">
            <div class="card-header">
              <div class="card-title">Year-by-Year Growth Schedule</div>
            </div>
            <div class="card-body">
              <div class="table-responsive">
                <table id="sip-breakdown-table">
                  <thead>
                    <tr>
                      <th>Year</th>
                      <th>Monthly SIP (₹)</th>
                      <th>Total Invested (₹)</th>
                      <th>Future Value (₹)</th>
                      <th>Wealth Gain (₹)</th>
                    </tr>
                  </thead>
                  <tbody id="sip-breakdown-tbody"></tbody>
                </table>
              </div>
            </div>
          </div>
        </div>

        <!-- TAB 2: GOAL RECOMMENDATION WIZARD -->
        <div id="tab-sip-wizard" class="sip-tab-pane hidden">
          <div class="wizard-box">
            <div style="margin-bottom:1.25rem;">
              <h3 style="margin-bottom:0.3rem;">Personalized Mutual Fund Portfolio Advisor</h3>
              <p class="text-muted" style="font-size:0.88rem;">
                Answer 5 questions to receive a tailored, transparent asset allocation plan with specific recommended Indian mutual fund schemes.
              </p>
            </div>

            <div class="wizard-grid">
              <div class="form-group">
                <label>1. Monthly Investment Budget (₹)</label>
                <input type="number" id="wiz-budget" value="10000" min="500" step="1000"/>
              </div>
              <div class="form-group">
                <label>2. Investment Horizon</label>
                <select id="wiz-horizon">
                  <option value="3">Short-Term (1 to 3 Years)</option>
                  <option value="5">Medium-Term (3 to 5 Years)</option>
                  <option value="7" selected>Long-Term (5 to 7 Years)</option>
                  <option value="10">Wealth Creation (7 to 10+ Years)</option>
                </select>
              </div>
              <div class="form-group">
                <label>3. Risk Tolerance</label>
                <select id="wiz-risk">
                  <option value="CONSERVATIVE">Conservative (Low volatility, high safety)</option>
                  <option value="MODERATE" selected>Moderate (Balanced equity & stability)</option>
                  <option value="AGGRESSIVE">Aggressive (High growth, maximum compounding)</option>
                </select>
              </div>
              <div class="form-group">
                <label>4. Primary Financial Goal</label>
                <select id="wiz-goal">
                  <option value="WEALTH_CREATION" selected>Long-Term Wealth Creation</option>
                  <option value="RETIREMENT">Retirement Corpus</option>
                  <option value="CHILD_EDUCATION">Child Higher Education</option>
                  <option value="HOUSE_PURCHASE">House Down-Payment</option>
                  <option value="EMERGENCY_FUND">Emergency Capital Fund</option>
                  <option value="TAX_SAVING">Tax Saving (Section 80C)</option>
                </select>
              </div>
            </div>

            <div class="form-group" style="flex-direction:row; align-items:center; gap:0.5rem; margin-bottom:1.5rem;">
              <input type="checkbox" id="wiz-tax" style="width:auto;"/>
              <label for="wiz-tax" style="cursor:pointer; margin-bottom:0;">
                I specifically require <strong>Section 80C Tax-Saving ELSS</strong> allocation (up to ₹1.5L annual deduction).
              </label>
            </div>

            <button class="btn btn-primary" onclick="Pages.runSIPRecommendation()">Generate Recommended Portfolio 🎯</button>

            <div id="wiz-recommendation-results" style="margin-top:1.5rem;" class="hidden"></div>
          </div>
        </div>

        <!-- TAB 3: MUTUAL FUND EXPLORER -->
        <div id="tab-sip-explorer" class="sip-tab-pane hidden">
          <div class="card">
            <div class="card-header" style="flex-wrap:wrap; gap:0.75rem;">
              <div class="pill-bar" style="margin-bottom:0;" id="mf-class-pills">
                <button class="pill-btn active" onclick="Pages.filterMFClass('ALL')">All Schemes</button>
                <button class="pill-btn" onclick="Pages.filterMFClass('EQUITY')">Equity Funds</button>
                <button class="pill-btn" onclick="Pages.filterMFClass('HYBRID')">Hybrid Funds</button>
                <button class="pill-btn" onclick="Pages.filterMFClass('DEBT')">Debt Funds</button>
                <button class="pill-btn" onclick="Pages.filterMFClass('INDEX')">Index & ETFs</button>
              </div>
              <div style="flex:1; max-width:260px;">
                <input type="text" id="mf-search-input" placeholder="🔍 Search fund name or AMC…" onkeyup="Pages.searchMFSchemes(this.value)"/>
              </div>
            </div>
            <div class="card-body">
              <div class="mf-grid" id="mf-schemes-container">
                <div class="spinner"></div>
              </div>
            </div>
          </div>
        </div>

        <!-- TAB 4: FUND COMPARISON -->
        <div id="tab-sip-comparison" class="sip-tab-pane hidden">
          <div class="card" style="margin-bottom:1rem;">
            <div class="card-header" style="justify-content:space-between; flex-wrap:wrap; gap:0.5rem;">
              <div>
                <div class="card-title">Multi-Scheme Mutual Fund Comparison</div>
                <div class="text-muted" style="font-size:0.83rem; margin-top:2px;">
                  Independently search, select, add and remove Indian mutual funds to compare CAGR returns, risk, expense ratio, NAV and AUM side-by-side.
                </div>
              </div>
              <div style="display:flex; gap:0.4rem;">
                <button class="btn btn-outline btn-sm" onclick="Pages.clearFundComparison()">✕ Clear All</button>
                <button class="btn btn-primary btn-sm" onclick="Pages.renderComparisonView()">↻ Refresh Comparison</button>
              </div>
            </div>
            <div class="card-body">
              <!-- Search & Quick-Add Bar -->
              <div style="position:relative; margin-bottom:1rem;">
                <input type="text" id="compare-fund-search" placeholder="🔍 Search fund name or AMC to add (e.g. Parag Parikh, HDFC, SBI, Index, Quant)..." oninput="Pages.searchCompareFunds(this.value)" autocomplete="off" style="width:100%;"/>
                <div id="compare-search-dropdown" style="display:none; position:absolute; top:100%; left:0; right:0; background:#fff; border:1px solid #cbd5e1; border-radius:6px; box-shadow:0 8px 24px rgba(0,0,0,0.12); z-index:100; max-height:280px; overflow-y:auto; margin-top:4px;"></div>
              </div>

              <!-- Selected Fund Chips -->
              <div style="display:flex; align-items:center; flex-wrap:wrap; gap:0.5rem;" id="compare-chips-container">
                <span class="text-muted" style="font-size:0.85rem; font-weight:600;">Selected Funds:</span>
                <span id="compare-chips-list" style="display:flex; flex-wrap:wrap; gap:0.4rem;"></span>
              </div>
            </div>
          </div>

          <!-- Comparison Table & Visual Analytics -->
          <div class="card">
            <div class="card-body" id="mf-comparison-container">
              <div class="text-center py-4 text-muted">
                <p>Select at least 2 mutual funds to see full side-by-side comparison.</p>
                <button class="btn btn-outline btn-sm mt-2" onclick="Pages.switchSIPTab('explorer')">Browse Funds Explorer</button>
              </div>
            </div>
          </div>
        </div>

        <!-- TAB 5: MY SIP PORTFOLIO -->
        <div id="tab-sip-portfolio" class="sip-tab-pane hidden">
          <div class="card">
            <div class="card-header" style="justify-content:space-between;">
              <div class="card-title">My Active SIP Commitments</div>
              <button class="btn btn-primary btn-sm" onclick="Pages.openSaveSIPModal()">+ Add New SIP Plan</button>
            </div>
            <div class="card-body" id="my-sips-container">
              <div class="spinner"></div>
            </div>
          </div>
        </div>

        <!-- Save SIP Modal -->
        <div class="modal-backdrop hidden" id="modal-save-sip">
          <div class="modal" style="max-width: 520px;">
            <div class="modal-header">
              <h3>Create Active SIP Plan</h3>
              <button class="modal-close" onclick="document.getElementById('modal-save-sip').classList.add('hidden')">✕</button>
            </div>
            <div class="modal-body">
              <div id="save-sip-error" class="alert alert-error hidden"></div>
              <div class="form-group">
                <label>Mutual Fund Scheme / Plan Name *</label>
                <input type="text" id="user-sip-name" placeholder="e.g. Parag Parikh Flexi Cap Fund" required/>
              </div>
              <div class="form-row">
                <div class="form-group">
                  <label>Monthly Contribution (₹) *</label>
                  <input type="number" id="user-sip-amount" value="5000" min="500" step="500" required/>
                </div>
                <div class="form-group">
                  <label>SIP Debit Day of Month</label>
                  <select id="user-sip-day">
                    <option value="1">1st of month</option>
                    <option value="5" selected>5th of month</option>
                    <option value="10">10th of month</option>
                    <option value="15">15th of month</option>
                    <option value="20">20th of month</option>
                    <option value="25">25th of month</option>
                  </select>
                </div>
              </div>
              <div class="form-row">
                <div class="form-group">
                  <label>Expected CAGR Return (%)</label>
                  <input type="number" id="user-sip-rate" value="12" min="1" max="30" step="0.5"/>
                </div>
                <div class="form-group">
                  <label>Tenure (Years)</label>
                  <input type="number" id="user-sip-years" value="5" min="1" max="30"/>
                </div>
              </div>
              <div class="form-group">
                <label>Annual Step-Up (%)</label>
                <input type="number" id="user-sip-stepup" value="5" min="0" max="30"/>
              </div>
              <button class="btn btn-primary btn-block mt-2" onclick="Pages.submitNewSIPPlan()">Save SIP to Portfolio</button>
            </div>
          </div>
        </div>
      `;

      container.innerHTML = html;
      this.runSIPCalc();
      this.loadMFSchemes();
      window._investsphere_compared_funds = window._investsphere_compared_funds || new Set();
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  renderSIP(container) {
    return this.renderSIPAndMutualFunds(container);
  },

  switchSIPTab(tabId) {
    document.querySelectorAll('#sip-main-tabs .sub-tab-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    document.querySelectorAll('.sip-tab-pane').forEach(p => p.classList.add('hidden'));
    const pane = document.getElementById(`tab-sip-${tabId}`);
    if (pane) pane.classList.remove('hidden');

    if (tabId === 'portfolio') {
      this.loadUserSIPs();
    } else if (tabId === 'comparison') {
      this.renderCompareChips();
      this.renderComparisonView();
    }
  },

  async runSIPCalc() {
    const monthly = parseFloat(document.getElementById('sip-calc-amount')?.value || 5000);
    const rate = parseFloat(document.getElementById('sip-calc-rate')?.value || 12);
    const years = parseInt(document.getElementById('sip-calc-years')?.value || 10, 10);
    const stepUp = parseFloat(document.getElementById('sip-calc-stepup')?.value || 0);

    const resBox = document.getElementById('sip-calc-result-box');
    const tbody = document.getElementById('sip-breakdown-tbody');
    if (!resBox) return;

    if (monthly <= 0 || rate <= 0 || years <= 0) {
      resBox.innerHTML = '<div class="alert alert-error">Please enter valid positive values.</div>';
      return;
    }

    try {
      const res = await Api.post('/mutual-funds/calculate-sip', {
        monthly_investment: monthly,
        annual_return_pct: rate,
        duration_years: years,
        step_up_pct: stepUp
      });

      const d = res.data || {};
      const invested = d.total_invested || 0;
      const futureVal = d.future_value || 0;
      const gain = d.wealth_gain || 0;
      const mult = d.wealth_multiplier || 1.0;

      resBox.innerHTML = `
        <div class="calc-result-box">
          <div class="calc-result-label">PROJECTED MATURITY CORPUS</div>
          <div class="calc-result-value">${formatINR(futureVal)}</div>
        </div>
        <div class="summary-grid" style="grid-template-columns: 1fr 1fr; margin-bottom: 0.5rem;">
          <div class="summary-card" style="padding:0.75rem;">
            <div class="summary-label">TOTAL INVESTED</div>
            <div class="summary-value" style="font-size:1.15rem;">${formatINR(invested)}</div>
          </div>
          <div class="summary-card card-success" style="padding:0.75rem;">
            <div class="summary-label">WEALTH GAINED</div>
            <div class="summary-value text-success" style="font-size:1.15rem;">+${formatINR(gain)}</div>
          </div>
        </div>
        <div class="mt-2 text-muted" style="font-size: 0.85rem;">
          Your invested capital grows by <strong>${mult}x</strong> over ${years} years with ${stepUp}% annual step-up.
        </div>
      `;

      if (d.year_wise_breakdown && tbody) {
        tbody.innerHTML = d.year_wise_breakdown.map(y => `
          <tr>
            <td><strong>Year ${y.year}</strong></td>
            <td>${formatINR(y.monthly_sip)}</td>
            <td>${formatINR(y.total_invested)}</td>
            <td class="font-bold">${formatINR(y.future_value)}</td>
            <td class="text-success font-bold">+${formatINR(y.wealth_gain)}</td>
          </tr>
        `).join('');
      }
    } catch (err) {
      resBox.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async runSIPRecommendation() {
    const budget = parseFloat(document.getElementById('wiz-budget')?.value || 10000);
    const horizon = parseInt(document.getElementById('wiz-horizon')?.value || 7, 10);
    const risk = document.getElementById('wiz-risk')?.value || 'MODERATE';
    const goal = document.getElementById('wiz-goal')?.value || 'WEALTH_CREATION';
    const taxSaving = document.getElementById('wiz-tax')?.checked || false;

    const resContainer = document.getElementById('wiz-recommendation-results');
    if (!resContainer) return;
    resContainer.classList.remove('hidden');
    resContainer.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.post('/mutual-funds/recommend', {
        monthly_budget: budget,
        horizon_years: horizon,
        risk_tolerance: risk,
        financial_goal: goal,
        tax_saving: taxSaving
      });

      const d = res.data || {};
      const allocs = d.allocations || [];
      const schemes = d.recommended_schemes || [];
      const proj = d.ten_year_projection || {};

      resContainer.innerHTML = `
        <div class="alert alert-success" style="margin-bottom:1.25rem;">
          <strong>🎯 Recommended Strategy:</strong> ${d.rationale}
        </div>

        <div style="font-weight:700; margin-bottom:0.6rem; font-size:0.95rem;">Target Portfolio Allocation (${formatINR(budget)}/month)</div>
        <div class="summary-grid" style="grid-template-columns: repeat(${allocs.length}, 1fr); margin-bottom:1.5rem;">
          ${allocs.map(a => `
            <div class="summary-card" style="padding:0.85rem;">
              <div class="summary-label">${a.category} (${a.pct}%)</div>
              <div class="summary-value" style="font-size:1.15rem; color:var(--primary);">${formatINR(a.amount)}</div>
              <div class="summary-sub text-muted">Monthly SIP</div>
            </div>
          `).join('')}
        </div>

        <div style="font-weight:700; margin-bottom:0.6rem; font-size:0.95rem;">Recommended Indian Mutual Fund Schemes</div>
        <div class="mf-grid" style="margin-bottom:1.5rem;">
          ${schemes.map(s => Pages.renderSchemeCard(s, false)).join('')}
        </div>

        <div style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:8px; padding:1.25rem;">
          <div style="font-weight:700; font-size:0.95rem; margin-bottom:0.3rem;">Projected Horizon Valuation</div>
          <div class="text-muted" style="font-size:0.85rem; margin-bottom:0.75rem;">
            Assuming disciplined investment and conservative ${d.expected_portfolio_cagr}% portfolio CAGR.
          </div>
          <div style="display:flex; justify-content:space-between; align-items:center;">
            <div>Total Capital Invested: <strong>${formatINR(proj.total_invested)}</strong></div>
            <div style="font-size:1.2rem; font-weight:700; color:var(--success);">Projected Corpus: ${formatINR(proj.future_value)}</div>
          </div>
        </div>
      `;
    } catch (err) {
      resContainer.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async loadMFSchemes() {
    const container = document.getElementById('mf-schemes-container');
    if (!container) return;
    try {
      const res = await Api.get('/mutual-funds');
      const schemes = res.data?.schemes || [];
      window._investsphere_all_schemes = schemes;
      container.innerHTML = schemes.map(s => Pages.renderSchemeCard(s, true)).join('');
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  renderSchemeCard(s, showCompareCheckbox = true) {
    const isCompared = window._investsphere_compared_funds?.has(s.id);
    return `
      <div class="mf-card" data-class="${s.asset_class}" data-name="${(s.name || '').toLowerCase()}" data-amc="${(s.amc || '').toLowerCase()}">
        <div>
          <div style="display:flex; justify-content:space-between; align-items:flex-start; margin-bottom:0.5rem;">
            <span class="badge badge-primary" style="font-size:0.72rem;">${s.category}</span>
            <span class="badge badge-neutral" style="font-size:0.72rem;">${s.risk} Risk</span>
          </div>
          <div style="font-weight:700; font-size:1rem; color:var(--text); line-height:1.35; margin-bottom:0.25rem;">${s.short_name || s.name}</div>
          <div class="text-muted" style="font-size:0.8rem; margin-bottom:0.6rem;">${s.amc} • Mgr: ${s.fund_manager || 'Team'}</div>

          <div class="mf-returns-row">
            <div><span class="text-muted">NAV:</span> <strong>₹${formatNumber(s.nav)}</strong></div>
            <div><span class="text-muted">1Y:</span> <strong class="text-success">+${s.returns_1y}%</strong></div>
            <div><span class="text-muted">3Y CAGR:</span> <strong class="text-success font-bold">+${s.returns_3y}%</strong></div>
          </div>

          <div style="display:flex; justify-content:space-between; font-size:0.8rem; color:var(--text-muted); margin-bottom:0.75rem;">
            <span>Expense Ratio: <strong>${s.expense_ratio}%</strong></span>
            <span>AUM: <strong>₹${formatNumber(s.aum_cr)} Cr</strong></span>
            <span>Min SIP: <strong>₹${s.min_sip}</strong></span>
          </div>
        </div>

        <div style="display:flex; justify-content:space-between; align-items:center; padding-top:0.6rem; border-top:1px solid #f1f5f9;">
          ${showCompareCheckbox ? `
            <label style="font-size:0.8rem; display:flex; align-items:center; gap:0.35rem; cursor:pointer;">
              <input type="checkbox" ${isCompared ? 'checked' : ''} onchange="Pages.toggleFundCompare('${s.id}')"/>
              <span>Compare</span>
            </label>
          ` : '<span></span>'}
          <button class="btn btn-outline btn-sm" onclick="Pages.prefillSaveSIP('${s.name}', ${s.min_sip || 1000}, ${s.returns_3y || 12})">+ Start SIP</button>
        </div>
      </div>
    `;
  },

  filterMFClass(cls) {
    document.querySelectorAll('#mf-class-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    const cards = document.querySelectorAll('#mf-schemes-container .mf-card');
    cards.forEach(c => {
      const cardClass = c.getAttribute('data-class');
      c.style.display = (cls === 'ALL' || cardClass === cls) ? '' : 'none';
    });
  },

  searchMFSchemes(query) {
    const q = query.toLowerCase().trim();
    const cards = document.querySelectorAll('#mf-schemes-container .mf-card');
    cards.forEach(c => {
      const name = c.getAttribute('data-name') || '';
      const amc = c.getAttribute('data-amc') || '';
      c.style.display = (!q || name.includes(q) || amc.includes(q)) ? '' : 'none';
    });
  },

  toggleFundCompare(schemeId) {
    if (!window._investsphere_compared_funds) {
      window._investsphere_compared_funds = new Set();
    }
    if (window._investsphere_compared_funds.has(schemeId)) {
      window._investsphere_compared_funds.delete(schemeId);
      if (typeof showToast === 'function') showToast('Fund removed from comparison.', 'info');
    } else {
      if (window._investsphere_compared_funds.size >= 5) {
        if (typeof showToast === 'function') {
          showToast('You can compare maximum 5 schemes at once.', 'warning');
        }
        return;
      }
      window._investsphere_compared_funds.add(schemeId);
      if (typeof showToast === 'function') showToast('Fund added to comparison.', 'success');
    }
    this.renderCompareChips();
  },

  async searchCompareFunds(query) {
    const dropdown = document.getElementById('compare-search-dropdown');
    if (!dropdown) return;
    const q = (query || '').toLowerCase().trim();
    if (!q) {
      dropdown.style.display = 'none';
      dropdown.innerHTML = '';
      return;
    }

    let schemes = window._investsphere_all_schemes;
    if (!schemes || schemes.length === 0) {
      try {
        const res = await Api.get('/mutual-funds');
        schemes = res.data?.schemes || [];
        window._investsphere_all_schemes = schemes;
      } catch (e) {
        schemes = [];
      }
    }

    const matched = (schemes || []).filter(s => {
      const name = (s.name || '').toLowerCase();
      const short = (s.short_name || '').toLowerCase();
      const amc = (s.amc || '').toLowerCase();
      const cat = (s.category || '').toLowerCase();
      return name.includes(q) || short.includes(q) || amc.includes(q) || cat.includes(q);
    }).slice(0, 8);

    if (matched.length === 0) {
      dropdown.innerHTML = '<div style="padding:0.75rem 1rem; color:var(--text-muted); font-size:0.85rem;">No mutual funds match your search.</div>';
      dropdown.style.display = 'block';
      return;
    }

    const compSet = window._investsphere_compared_funds || new Set();

    dropdown.innerHTML = matched.map(s => {
      const isAdded = compSet.has(s.id);
      return `
        <div style="display:flex; justify-content:space-between; align-items:center; padding:0.6rem 1rem; border-bottom:1px solid #f1f5f9; hover:background:#f8fafc;">
          <div>
            <div style="font-weight:600; font-size:0.88rem; color:var(--text);">${s.short_name || s.name}</div>
            <div class="text-muted" style="font-size:0.78rem;">
              ${s.amc} • <span class="badge badge-primary" style="font-size:0.7rem; padding:1px 5px;">${s.category}</span> • 3Y: <span class="text-success font-bold">+${s.returns_3y}%</span>
            </div>
          </div>
          <div>
            ${isAdded ?
              `<button class="btn btn-outline btn-sm text-danger" onclick="Pages.removeFundFromCompare('${s.id}')">✕ Remove</button>` :
              `<button class="btn btn-primary btn-sm" onclick="Pages.addFundToCompare('${s.id}')">+ Add</button>`
            }
          </div>
        </div>
      `;
    }).join('');
    dropdown.style.display = 'block';
  },

  addFundToCompare(schemeId) {
    if (!window._investsphere_compared_funds) {
      window._investsphere_compared_funds = new Set();
    }
    if (window._investsphere_compared_funds.size >= 5) {
      if (typeof showToast === 'function') {
        showToast('Maximum 5 schemes can be compared simultaneously.', 'warning');
      }
      return;
    }
    window._investsphere_compared_funds.add(schemeId);
    const searchInput = document.getElementById('compare-fund-search');
    if (searchInput) searchInput.value = '';
    const dropdown = document.getElementById('compare-search-dropdown');
    if (dropdown) dropdown.style.display = 'none';

    if (typeof showToast === 'function') {
      showToast('Fund added to comparison.', 'success');
    }
    this.renderCompareChips();
    this.renderComparisonView();
  },

  removeFundFromCompare(schemeId) {
    if (window._investsphere_compared_funds) {
      window._investsphere_compared_funds.delete(schemeId);
    }
    this.renderCompareChips();
    this.renderComparisonView();
  },

  clearFundComparison() {
    window._investsphere_compared_funds = new Set();
    this.renderCompareChips();
    this.renderComparisonView();
    if (typeof showToast === 'function') {
      showToast('Fund comparison list cleared.', 'info');
    }
  },

  renderCompareChips() {
    const listEl = document.getElementById('compare-chips-list');
    if (!listEl) return;
    const ids = Array.from(window._investsphere_compared_funds || []);
    if (ids.length === 0) {
      listEl.innerHTML = '<span class="text-muted" style="font-size:0.83rem; font-style:italic;">None selected yet. Search above or check "Compare" in the Explorer.</span>';
      return;
    }

    const schemes = window._investsphere_all_schemes || [];
    listEl.innerHTML = ids.map(id => {
      const s = schemes.find(x => x.id === id);
      const label = s ? (s.short_name || s.name) : id;
      return `
        <span style="display:inline-flex; align-items:center; gap:0.35rem; background:#eff6ff; color:#1e40af; border:1px solid #bfdbfe; border-radius:18px; padding:0.25rem 0.65rem; font-size:0.82rem; font-weight:600;">
          ${label}
          <span style="cursor:pointer; color:#ef4444; font-weight:bold; margin-left:2px;" onclick="Pages.removeFundFromCompare('${id}')" title="Remove">✕</span>
        </span>
      `;
    }).join('');
  },

  async renderComparisonView() {
    const container = document.getElementById('mf-comparison-container');
    if (!container) return;
    const ids = Array.from(window._investsphere_compared_funds || []);
    this.renderCompareChips();

    if (ids.length < 2) {
      container.innerHTML = `
        <div class="text-center py-4 text-muted">
          <div style="font-size:2.2rem; margin-bottom:0.5rem;">⚖️</div>
          <h3 style="margin-bottom:0.3rem;">Compare Multiple Mutual Funds</h3>
          <p style="max-width:500px; margin:0 auto 1rem auto; font-size:0.88rem;">
            Select at least 2 mutual funds using the search box above or the "Compare" checkboxes in the Explorer tab to review side-by-side returns, CAGR, risk, expense ratio, and portfolio holdings.
          </p>
          <div style="display:flex; justify-content:center; gap:0.5rem;">
            <button class="btn btn-outline btn-sm" onclick="Pages.addFundToCompare('ppfas-flexi-cap'); Pages.addFundToCompare('hdfc-flexi-cap');">
              + Compare Flexi Cap Leaders
            </button>
            <button class="btn btn-outline btn-sm" onclick="Pages.addFundToCompare('uti-nifty-50-index'); Pages.addFundToCompare('mirae-large-cap');">
              + Compare Large Cap & Index
            </button>
          </div>
        </div>
      `;
      return;
    }

    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.post('/mutual-funds/compare', { scheme_ids: ids });
      const schemes = res.data || [];

      if (schemes.length === 0) {
        container.innerHTML = '<div class="alert alert-error">Unable to load scheme details for comparison.</div>';
        return;
      }

      container.innerHTML = `
        <!-- Visual Returns Bar Comparison -->
        <div style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:8px; padding:1.25rem; margin-bottom:1.5rem;">
          <div style="font-weight:700; font-size:0.95rem; margin-bottom:0.75rem;">Performance Returns Comparison Across Schemes</div>
          <div style="display:grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap:1rem;">
            ${schemes.map(s => `
              <div style="background:#fff; border:1px solid #cbd5e1; border-radius:6px; padding:0.85rem;">
                <div style="font-weight:700; font-size:0.9rem; color:var(--text); margin-bottom:0.5rem; line-height:1.25;">
                  ${s.short_name || s.name}
                </div>
                <div style="font-size:0.8rem; margin-bottom:0.3rem; display:flex; justify-content:space-between;">
                  <span class="text-muted">1-Year Return:</span>
                  <strong class="text-success">+${s.returns_1y !== undefined ? s.returns_1y : '—'}%</strong>
                </div>
                <div style="font-size:0.8rem; margin-bottom:0.3rem; display:flex; justify-content:space-between;">
                  <span class="text-muted">3-Year CAGR:</span>
                  <strong class="text-success font-bold">+${s.returns_3y !== undefined ? s.returns_3y : '—'}%</strong>
                </div>
                <div style="font-size:0.8rem; display:flex; justify-content:space-between;">
                  <span class="text-muted">5-Year CAGR:</span>
                  <strong class="text-success">+${s.returns_5y !== undefined ? s.returns_5y : '—'}%</strong>
                </div>
              </div>
            `).join('')}
          </div>
        </div>

        <!-- Detailed Side-by-Side Comparison Table -->
        <div class="table-responsive">
          <table class="compare-table" style="width:100%; border-collapse:collapse;">
            <thead>
              <tr>
                <th style="width:220px; background:#f8fafc;">Metric / Attribute</th>
                ${schemes.map(s => `
                  <th>
                    <div style="display:flex; justify-content:space-between; align-items:flex-start; gap:0.5rem;">
                      <div>
                        <strong>${s.short_name || s.name}</strong>
                        <div style="font-size:0.75rem; color:var(--text-muted); font-weight:normal;">${s.amc}</div>
                      </div>
                      <button class="btn btn-outline btn-sm text-danger" style="padding:1px 6px; font-size:0.75rem;" onclick="Pages.removeFundFromCompare('${s.id}')" title="Remove from comparison">✕</button>
                    </div>
                  </th>
                `).join('')}
              </tr>
            </thead>
            <tbody>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Category</td>
                ${schemes.map(s => `<td><span class="badge badge-primary">${s.category || '—'}</span></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Asset Class</td>
                ${schemes.map(s => `<td><span class="badge badge-neutral">${s.asset_class || '—'}</span></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Fund House (AMC)</td>
                ${schemes.map(s => `<td>${s.amc || '—'}</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Current NAV</td>
                ${schemes.map(s => `<td><strong>₹${formatNumber(s.nav)}</strong> <small class="text-muted">(${s.nav_date || 'Live'})</small></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">1-Year Return</td>
                ${schemes.map(s => `<td class="text-success font-bold">+${s.returns_1y !== undefined ? s.returns_1y : '—'}%</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">3-Year CAGR</td>
                ${schemes.map(s => `<td class="text-success font-bold">+${s.returns_3y !== undefined ? s.returns_3y : '—'}%</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">5-Year CAGR</td>
                ${schemes.map(s => `<td class="text-success font-bold">+${s.returns_5y !== undefined ? s.returns_5y : '—'}%</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">10-Year CAGR</td>
                ${schemes.map(s => `<td>${s.returns_10y !== undefined ? `<span class="text-success font-bold">+${s.returns_10y}%</span>` : '<span class="text-muted">—</span>'}</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Expense Ratio</td>
                ${schemes.map(s => `<td><strong>${s.expense_ratio !== undefined ? s.expense_ratio + '%' : '—'}</strong></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Total AUM (Assets)</td>
                ${schemes.map(s => `<td>₹${formatNumber(s.aum_cr)} Cr</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Minimum Monthly SIP</td>
                ${schemes.map(s => `<td>₹${formatNumber(s.min_sip)}</td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">SEBI Riskometer</td>
                ${schemes.map(s => `<td><span class="badge badge-warning">${s.risk || 'Moderate'}</span></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Morningstar Rating</td>
                ${schemes.map(s => `<td>${'★'.repeat(s.rating || 4)} <small class="text-muted">(${s.rating || 4}/5)</small></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Benchmark Index</td>
                ${schemes.map(s => `<td><small>${s.benchmark || 'NIFTY 500 TRI'}</small></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Fund Manager(s)</td>
                ${schemes.map(s => `<td><small>${s.fund_manager || 'Investment Team'}</small></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Top Holdings</td>
                ${schemes.map(s => `<td><small class="text-muted">${(s.top_holdings || []).join(', ') || 'Diversified Equity'}</small></td>`).join('')}
              </tr>
              <tr>
                <td style="font-weight:600; background:#f8fafc;">Action</td>
                ${schemes.map(s => `
                  <td>
                    <button class="btn btn-primary btn-sm" onclick="Pages.prefillSaveSIP('${s.name.replace(/'/g, "\\'")}', ${s.min_sip || 1000}, ${s.returns_3y || 12})">
                      + Start SIP
                    </button>
                  </td>
                `).join('')}
              </tr>
            </tbody>
          </table>
        </div>
      `;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to compare schemes: ${err.message}</div>`;
    }
  },

  async loadUserSIPs() {
    const container = document.getElementById('my-sips-container');
    if (!container) return;
    container.innerHTML = '<div class="spinner"></div>';

    try {
      const res = await Api.get('/mutual-funds/my-sips');
      const plans = res.data || [];
      if (plans.length === 0) {
        container.innerHTML = `
          <div class="text-center py-4 text-muted">
            <p>You have not recorded any active SIP plans yet.</p>
            <button class="btn btn-primary btn-sm mt-2" onclick="Pages.openSaveSIPModal()">+ Add Your First SIP</button>
          </div>
        `;
        return;
      }

      container.innerHTML = `
        <div class="table-responsive">
          <table>
            <thead>
              <tr>
                <th>Scheme / Plan Name</th>
                <th>Monthly Contribution</th>
                <th>Debit Day</th>
                <th>Expected Return</th>
                <th>Annual Step-Up</th>
                <th>Projected Maturity</th>
                <th>Status</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              ${plans.map(p => `
                <tr>
                  <td><strong>${p.plan_name}</strong></td>
                  <td class="font-bold">${formatINR(p.monthly_amount)}</td>
                  <td>${p.contribution_day}th of month</td>
                  <td>${p.expected_annual_return}% p.a.</td>
                  <td>${p.step_up_percent}%</td>
                  <td class="text-success font-bold">${formatINR(p.projected_future_value)}</td>
                  <td><span class="badge ${p.status === 'ACTIVE' ? 'badge-success' : 'badge-warning'}">${p.status}</span></td>
                  <td>
                    ${p.status === 'ACTIVE' ?
                      `<button class="btn btn-outline btn-sm" onclick="Pages.updateUserSIPStatus(${p.sip_plan_id}, 'PAUSED')">Pause</button>` :
                      `<button class="btn btn-success btn-sm" onclick="Pages.updateUserSIPStatus(${p.sip_plan_id}, 'ACTIVE')">Resume</button>`
                    }
                  </td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
      `;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  openSaveSIPModal() {
    document.getElementById('modal-save-sip')?.classList.remove('hidden');
    document.getElementById('save-sip-error')?.classList.add('hidden');
  },

  prefillSaveSIP(name, amount, rate) {
    this.openSaveSIPModal();
    const nameEl = document.getElementById('user-sip-name');
    const amtEl = document.getElementById('user-sip-amount');
    const rateEl = document.getElementById('user-sip-rate');
    if (nameEl) nameEl.value = name;
    if (amtEl) amtEl.value = amount || 5000;
    if (rateEl) rateEl.value = rate || 12;
  },

  async submitNewSIPPlan() {
    const name = document.getElementById('user-sip-name')?.value.trim();
    const amount = parseFloat(document.getElementById('user-sip-amount')?.value || 0);
    const day = parseInt(document.getElementById('user-sip-day')?.value || 5, 10);
    const rate = parseFloat(document.getElementById('user-sip-rate')?.value || 12);
    const years = parseFloat(document.getElementById('user-sip-years')?.value || 5);
    const stepUp = parseFloat(document.getElementById('user-sip-stepup')?.value || 0);
    const errBox = document.getElementById('save-sip-error');

    if (errBox) errBox.classList.add('hidden');

    if (!name || amount < 500) {
      if (errBox) {
        errBox.textContent = 'Please enter a valid scheme name and minimum ₹500 monthly amount.';
        errBox.classList.remove('hidden');
      }
      return;
    }

    try {
      await Api.post('/mutual-funds/my-sips', {
        plan_name: name,
        monthly_amount: amount,
        expected_annual_return: rate,
        duration_years: years,
        step_up_percent: stepUp,
        contribution_day: day
      });
      document.getElementById('modal-save-sip')?.classList.add('hidden');
      if (typeof showToast === 'function') {
        showToast('SIP plan saved successfully!', 'success');
      }
      this.loadUserSIPs();
    } catch (err) {
      if (errBox) {
        errBox.textContent = err.message;
        errBox.classList.remove('hidden');
      }
    }
  },

  async updateUserSIPStatus(planId, status) {
    try {
      await Api.patch(`/mutual-funds/my-sips/${planId}/status`, { status });
      if (typeof showToast === 'function') {
        showToast(`SIP plan marked as ${status}.`, 'info');
      }
      this.loadUserSIPs();
    } catch (err) {
      if (typeof showToast === 'function') {
        showToast(err.message, 'error');
      }
    }
  },

  // ─── SWP CALCULATOR ────────────────────────────────────────────────────────
  renderSWP(container) {
    container.innerHTML = `
      <div class="page-header">
        <h2>Systematic Withdrawal Plan (SWP) Calculator</h2>
      </div>

      <div class="calc-layout">
        <!-- Input Form -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">Withdrawal Parameters</div>
          </div>
          <div class="card-body">
            <div class="form-group">
              <label>Initial Investment Corpus (₹) *</label>
              <input type="number" id="swp-corpus" value="1000000" min="10000" step="50000"/>
            </div>
            <div class="form-group">
              <label>Monthly Withdrawal Amount (₹) *</label>
              <input type="number" id="swp-withdrawal" value="8000" min="500" step="500"/>
            </div>
            <div class="form-group">
              <label>Expected Annual Return Rate (%) *</label>
              <input type="number" id="swp-rate" value="9" min="1" max="30" step="0.5"/>
            </div>
            <div class="form-group">
              <label>Withdrawal Period (Years) *</label>
              <input type="number" id="swp-years" value="10" min="1" max="40" step="1"/>
            </div>
            <button class="btn btn-primary btn-block mt-2" onclick="Pages.calculateSWP()">Calculate Cashflow</button>
          </div>
        </div>

        <!-- Output Results -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">Cashflow Summary</div>
          </div>
          <div class="card-body" id="swp-results-box">
            <p class="text-muted">Click "Calculate Cashflow" to analyze corpus sustainability.</p>
          </div>
        </div>
      </div>

      <!-- Schedule Table -->
      <div class="card mt-3 hidden" id="swp-schedule-card">
        <div class="card-header">
          <div class="card-title">Annual Corpus & Withdrawal Schedule</div>
        </div>
        <div class="card-body">
          <div class="table-responsive">
            <table id="swp-schedule-table">
              <thead>
                <tr>
                  <th>Year</th>
                  <th>Opening Balance (₹)</th>
                  <th>Withdrawn This Year (₹)</th>
                  <th>Interest Earned (₹)</th>
                  <th>Closing Balance (₹)</th>
                </tr>
              </thead>
              <tbody></tbody>
            </table>
          </div>
        </div>
      </div>
    `;

    this.calculateSWP();
  },

  async calculateSWP() {
    const initial_corpus = parseFloat(document.getElementById('swp-corpus')?.value || 0);
    const monthly_withdrawal = parseFloat(document.getElementById('swp-withdrawal')?.value || 0);
    const annual_rate = parseFloat(document.getElementById('swp-rate')?.value || 0);
    const tenure_years = parseInt(document.getElementById('swp-years')?.value || 0, 10);

    const resBox = document.getElementById('swp-results-box');
    const schedCard = document.getElementById('swp-schedule-card');
    const schedTable = document.querySelector('#swp-schedule-table tbody');

    if (initial_corpus <= 0 || monthly_withdrawal <= 0 || tenure_years <= 0) {
      if (resBox) resBox.innerHTML = '<div class="alert alert-error">Please enter valid positive values.</div>';
      return;
    }

    try {
      const res = await Api.post('/calculators/swp', {
        initial_corpus,
        monthly_withdrawal,
        annual_return_percent: annual_rate,
        duration_years: tenure_years
      });

      const d = res.data || {};
      const withdrawn = d.total_withdrawn || 0;
      const finalBal = d.final_balance || 0;
      const depleted = d.depleted;

      if (resBox) {
        resBox.innerHTML = `
          <div class="calc-result-box" style="${depleted ? 'background: #fef2f2; border-color: #fecaca;' : ''}">
            <div class="calc-result-label" style="${depleted ? 'color: var(--danger);' : ''}">Estimated Final Balance</div>
            <div class="calc-result-value" style="${depleted ? 'color: var(--danger);' : ''}">${formatINR(finalBal)}</div>
          </div>
          <div class="summary-grid" style="grid-template-columns: 1fr 1fr; margin-bottom: 0;">
            <div class="summary-card">
              <div class="summary-label">Initial Corpus</div>
              <div class="summary-value" style="font-size:1.15rem;">${formatINR(initial_corpus)}</div>
            </div>
            <div class="summary-card card-primary">
              <div class="summary-label">Total Withdrawn</div>
              <div class="summary-value" style="font-size:1.15rem;">${formatINR(withdrawn)}</div>
            </div>
          </div>
          ${depleted ? `
            <div class="alert alert-error mt-2">
              ⚠ Warning: The corpus was depleted in <strong>Year ${d.depleted_at_year || '—'}</strong> before the full ${tenure_years} years.
            </div>
          ` : `
            <div class="alert alert-success mt-2">
              ✓ Corpus remains solvent after ${tenure_years} years of monthly withdrawals.
            </div>
          `}
        `;
      }

      if (d.yearly_breakdown && d.yearly_breakdown.length > 0 && schedCard && schedTable) {
        schedTable.innerHTML = d.yearly_breakdown.map(s => `
          <tr>
            <td><strong>Year ${s.year}</strong></td>
            <td>${formatINR(s.opening_balance)}</td>
            <td>${formatINR(s.total_withdrawn_this_year || 0)}</td>
            <td class="text-success font-bold">+${formatINR(s.interest_earned || 0)}</td>
            <td class="font-bold">${formatINR(s.balance_at_year_end)}</td>
          </tr>
        `).join('');
        schedCard.classList.remove('hidden');
      }
    } catch (err) {
      if (resBox) resBox.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  // ─── MATURITY TRACKER ──────────────────────────────────────────────────────
  async renderMaturity(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/maturity');
      const data = res.data || {};
      const events = data.events || [];
      const sum = data.summary || {};

      let html = `
        <div class="page-header">
          <div>
            <h2>Fixed Income & Maturity Tracker</h2>
            <p class="text-muted" style="font-size:0.88rem; margin-top:2px;">
              Auto-synced tracking for Bank Fixed Deposits, Corporate Bonds, and Government Securities with instrument-specific interest accrual and maturity timelines.
            </p>
          </div>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="App.refreshCurrentPage()">↻ Refresh Schedule</button>
            <button class="btn btn-primary btn-sm" onclick="Modals.open('modal-add-investment')">+ Add Maturity Asset</button>
          </div>
        </div>

        <!-- Summary Metric Cards -->
        <div class="summary-grid" style="grid-template-columns: repeat(4, 1fr); gap: 1rem; margin-bottom: 1.5rem;">
          <div class="summary-card ${sum.overdue_count > 0 ? 'card-danger' : ''}">
            <div class="summary-label">OVERDUE / MATURED</div>
            <div class="summary-value ${sum.overdue_count > 0 ? 'text-danger' : ''}" style="font-size:1.45rem;">${sum.overdue_count || 0}</div>
            <div class="summary-sub">${sum.overdue_count > 0 ? 'Immediate action required' : 'No overdue deposits'}</div>
          </div>
          <div class="summary-card ${sum.due_30_count > 0 ? 'card-warning' : ''}">
            <div class="summary-label">MATURING IN 30 DAYS</div>
            <div class="summary-value" style="font-size:1.45rem;">${sum.due_30_count || 0}</div>
            <div class="summary-sub text-muted">${formatINR(sum.due_30_amount || 0)}</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">MATURING IN 90 DAYS</div>
            <div class="summary-value" style="font-size:1.45rem;">${sum.due_90_count || 0}</div>
            <div class="summary-sub text-muted">${formatINR(sum.due_90_amount || 0)}</div>
          </div>
          <div class="summary-card card-primary">
            <div class="summary-label">TOTAL MATURITY VALUE</div>
            <div class="summary-value font-bold" style="font-size:1.45rem;">${formatINR(sum.total_maturity_amount || 0)}</div>
            <div class="summary-sub text-muted">${events.length} tracked instruments</div>
          </div>
        </div>

        <!-- Filter & Search Controls -->
        <div class="filter-bar" style="margin-bottom:1rem; flex-wrap:wrap; gap:0.5rem; justify-content:space-between;">
          <div class="pill-bar" style="margin-bottom:0;" id="maturity-filter-pills">
            <button class="pill-btn active" onclick="Pages.filterMaturityTable('ALL')">All (${events.length})</button>
            <button class="pill-btn" onclick="Pages.filterMaturityTable('UPCOMING')">Active Upcoming</button>
            <button class="pill-btn" onclick="Pages.filterMaturityTable('DUE30')">Due in 30 Days (${sum.due_30_count || 0})</button>
            <button class="pill-btn" onclick="Pages.filterMaturityTable('OVERDUE')">Overdue (${sum.overdue_count || 0})</button>
            <button class="pill-btn" onclick="Pages.filterMaturityTable('COMPLETED')">Settled / Completed</button>
          </div>
          <div style="min-width:240px;">
            <input type="text" id="maturity-search-input" placeholder="🔍 Search bank or bond issuer..." oninput="Pages.searchMaturityTable(this.value)"/>
          </div>
        </div>

        <div class="card">
          <div class="card-body">
            ${events.length === 0 ? `
              <div class="empty-state">
                <span class="empty-icon">📅</span>
                <h3>No Maturity-Based Investments Detected</h3>
                <p>Add fixed deposits or bonds with maturity dates in your Portfolio or via "+ Add Maturity Asset". They will automatically appear here with compound interest calculations.</p>
                <button class="btn btn-primary mt-2" onclick="Modals.open('modal-add-investment')">+ Add Fixed Deposit / Bond</button>
              </div>
            ` : `
              <div class="table-responsive">
                <table id="maturity-main-table">
                  <thead>
                    <tr>
                      <th>Institution / Issuer</th>
                      <th>Category</th>
                      <th>Interest / Coupon</th>
                      <th>Maturity Date</th>
                      <th>Timeline</th>
                      <th>Principal (₹)</th>
                      <th>Expected Gain (₹)</th>
                      <th>Maturity Value (₹)</th>
                      <th>Status</th>
                      <th>Actions</th>
                    </tr>
                  </thead>
                  <tbody id="maturity-table-body">
                    ${events.map(e => {
                      const days = e.days_remaining !== undefined ? e.days_remaining : 0;
                      const isProcessed = e.status === 'PROCESSED';
                      let badgeCls = 'badge-success';
                      let label = `${days} days left`;
                      let filterGroup = 'UPCOMING';

                      if (isProcessed) {
                        badgeCls = 'badge-neutral';
                        label = 'Settled';
                        filterGroup = 'COMPLETED';
                      } else if (days < 0) {
                        badgeCls = 'badge-danger';
                        label = `${Math.abs(days)}d Overdue`;
                        filterGroup = 'OVERDUE';
                      } else if (days <= 30) {
                        badgeCls = 'badge-warning';
                        label = `⚡ ${days}d Left`;
                        filterGroup = 'DUE30';
                      }

                      const rateLabel = e.interest_rate ? `${e.interest_rate}% p.a.` : '—';
                      const freqLabel = e.compounding_frequency ? `<small class="text-muted">(${e.compounding_frequency})</small>` : (e.coupon_frequency ? `<small class="text-muted">(${e.coupon_frequency})</small>` : '');
                      const expGain = e.expected_returns !== undefined ? e.expected_returns : (e.maturity_amount - e.principal_amount);

                      return `
                        <tr data-filter="${filterGroup}" data-status="${e.status}" data-search="${((e.institution_name || '') + ' ' + (e.instrument_name || '')).toLowerCase()}">
                          <td>
                            <strong>${e.institution_name || e.instrument_name || '—'}</strong>
                            ${e.description ? `<br><small class="text-muted">${e.description}</small>` : ''}
                          </td>
                          <td>${getCategoryBadge(e.type_code || 'FIXED_DEPOSIT')}</td>
                          <td>
                            <strong>${rateLabel}</strong> ${freqLabel}
                          </td>
                          <td class="font-bold">${formatDate(e.maturity_date || e.event_date)}</td>
                          <td><span class="badge ${badgeCls}">${label}</span></td>
                          <td>${formatINR(e.principal_amount)}</td>
                          <td class="text-success font-bold">${expGain > 0 ? '+' + formatINR(expGain) : '—'}</td>
                          <td class="font-bold" style="color:var(--primary); font-size:1.02rem;">${formatINR(e.maturity_amount || e.principal_amount)}</td>
                          <td>
                            <span class="badge ${isProcessed ? 'badge-neutral' : (days < 0 ? 'badge-danger' : 'badge-info')}">
                              ${isProcessed ? 'COMPLETED' : (days < 0 ? 'OVERDUE' : e.status)}
                            </span>
                          </td>
                          <td>
                            <div style="display:flex; gap:0.3rem;">
                              ${isProcessed ?
                                `<button class="btn btn-outline btn-sm" onclick="Pages.updateMaturityStatus(${e.event_id}, 'PENDING')" title="Re-open into active tracking">Re-open</button>` :
                                `<button class="btn btn-success btn-sm" onclick="Pages.updateMaturityStatus(${e.event_id}, 'PROCESSED')" title="Mark Matured & Settled">✓ Settle</button>`
                              }
                            </div>
                          </td>
                        </tr>
                      `;
                    }).join('')}
                  </tbody>
                </table>
              </div>
            `}
          </div>
        </div>
      `;

      container.innerHTML = html;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load maturity schedule: ${err.message}</div>`;
    }
  },

  async updateMaturityStatus(eventId, newStatus) {
    try {
      const res = await Api.put(`/maturity/${eventId}`, { status: newStatus });
      if (res.success || res.status === 'ok') {
        if (typeof showToast === 'function') {
          showToast(`Maturity event marked as ${newStatus}.`, 'success');
        }
        App.refreshCurrentPage();
      } else {
        throw new Error(res.error || 'Failed to update status');
      }
    } catch (err) {
      if (typeof showToast === 'function') {
        showToast(err.message, 'error');
      }
    }
  },

  filterMaturityTable(filter) {
    document.querySelectorAll('#maturity-filter-pills .pill-btn').forEach(btn => btn.classList.remove('active'));
    event?.target?.classList.add('active');

    const rows = document.querySelectorAll('#maturity-table-body tr');
    rows.forEach(r => {
      const group = r.getAttribute('data-filter');
      if (filter === 'ALL') {
        r.style.display = '';
      } else if (filter === 'UPCOMING') {
        r.style.display = (group === 'UPCOMING' || group === 'DUE30') ? '' : 'none';
      } else if (filter === 'DUE30') {
        r.style.display = group === 'DUE30' ? '' : 'none';
      } else if (filter === 'OVERDUE') {
        r.style.display = group === 'OVERDUE' ? '' : 'none';
      } else if (filter === 'COMPLETED') {
        r.style.display = group === 'COMPLETED' ? '' : 'none';
      }
    });
  },

  searchMaturityTable(query) {
    const q = (query || '').toLowerCase().trim();
    const rows = document.querySelectorAll('#maturity-table-body tr');
    rows.forEach(r => {
      const text = r.getAttribute('data-search') || '';
      r.style.display = (!q || text.includes(q)) ? '' : 'none';
    });
  },

  // ─── ANALYTICS ─────────────────────────────────────────────────────────────
  async renderAnalytics(container) {
    container.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/analytics');
      const d = res.data || {};
      const alloc = d.allocation || [];
      const topP = d.top_performers || [];
      const worstP = d.worst_performers || [];
      const positions = d.positions || [];

      let html = `
        <div class="page-header">
          <h2>Portfolio Analytics & Performance</h2>
          <div class="topbar-actions">
            <button class="btn btn-outline btn-sm" onclick="Api.download('/reports/pnl?format=csv', 'pnl_report.csv')">Export P&L CSV</button>
          </div>
        </div>

        <div class="summary-grid">
          <div class="summary-card card-primary">
            <div class="summary-label">Total Portfolio Value</div>
            <div class="summary-value">${formatINR(d.total_current_value || 0)}</div>
          </div>
          <div class="summary-card">
            <div class="summary-label">Total Cost Basis</div>
            <div class="summary-value">${formatINR(d.total_invested || 0)}</div>
          </div>
          <div class="summary-card ${(d.total_gain_loss || 0) >= 0 ? 'card-success' : 'card-danger'}">
            <div class="summary-label">Net Gain / Loss</div>
            <div class="summary-value ${(d.total_gain_loss || 0) >= 0 ? 'text-success' : 'text-danger'}">
              ${(d.total_gain_loss || 0) >= 0 ? '+' : ''}${formatINR(d.total_gain_loss || 0)}
            </div>
            <div class="summary-sub">${formatNumber(d.total_gain_loss_pct || 0)}% Return</div>
          </div>
        </div>

        <div class="content-grid">
          <!-- Top Performers -->
          <div class="card">
            <div class="card-header">
              <div class="card-title">Top Performers</div>
            </div>
            <div class="card-body">
              ${topP.length === 0 ? '<p class="text-muted">No performer data available.</p>' :
                `<div class="table-responsive">
                  <table>
                    <thead><tr><th>Instrument</th><th>Value</th><th>Gain</th></tr></thead>
                    <tbody>
                      ${topP.map(p => `
                        <tr>
                          <td><strong>${p.name || p.symbol}</strong></td>
                          <td>${formatINR(p.current_value)}</td>
                          <td class="text-success font-bold">+${formatINR(p.unrealized_gain_loss)} <small>(+${formatNumber(p.return_pct)}%)</small></td>
                        </tr>
                      `).join('')}
                    </tbody>
                  </table>
                </div>`
              }
            </div>
          </div>

          <!-- Worst Performers -->
          <div class="card">
            <div class="card-header">
              <div class="card-title">Underperformers</div>
            </div>
            <div class="card-body">
              ${worstP.length === 0 ? '<p class="text-muted">No underperforming positions.</p>' :
                `<div class="table-responsive">
                  <table>
                    <thead><tr><th>Instrument</th><th>Value</th><th>Gain / Loss</th></tr></thead>
                    <tbody>
                      ${worstP.map(p => `
                        <tr>
                          <td><strong>${p.name || p.symbol}</strong></td>
                          <td>${formatINR(p.current_value)}</td>
                          <td class="${p.unrealized_gain_loss >= 0 ? 'text-success' : 'text-danger'} font-bold">
                            ${p.unrealized_gain_loss >= 0 ? '+' : ''}${formatINR(p.unrealized_gain_loss)} <small>(${formatNumber(p.return_pct)}%)</small>
                          </td>
                        </tr>
                      `).join('')}
                    </tbody>
                  </table>
                </div>`
              }
            </div>
          </div>
        </div>

        <!-- Positions Table -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">All Positions Performance Breakdown</div>
          </div>
          <div class="card-body">
            <div class="table-responsive">
              <table>
                <thead>
                  <tr>
                    <th>Category</th>
                    <th>Name</th>
                    <th>Cost (₹)</th>
                    <th>Current Value (₹)</th>
                    <th>P&L (₹)</th>
                    <th>Return %</th>
                    <th>Allocation</th>
                  </tr>
                </thead>
                <tbody>
                  ${positions.map(p => {
                    const gl = p.unrealized_gain_loss || 0;
                    const glClass = gl >= 0 ? 'text-success' : 'text-danger';
                    const sign = gl >= 0 ? '+' : '';
                    return `
                      <tr>
                        <td>${getCategoryBadge(p.category || p.type_code)}</td>
                        <td><strong>${p.name}</strong></td>
                        <td>${formatINR(p.cost_basis || p.total_invested)}</td>
                        <td class="font-bold">${formatINR(p.current_value)}</td>
                        <td class="${glClass} font-bold">${sign}${formatINR(gl)}</td>
                        <td class="${glClass} font-bold">${sign}${formatNumber(p.return_pct)}%</td>
                        <td>${formatNumber(p.allocation_pct || 0)}%</td>
                      </tr>
                    `;
                  }).join('')}
                </tbody>
              </table>
            </div>
          </div>
        </div>
      `;

      container.innerHTML = html;
    } catch (err) {
      container.innerHTML = `<div class="alert alert-error">Failed to load analytics: ${err.message}</div>`;
    }
  },

  // ─── REPORTS ───────────────────────────────────────────────────────────────
  async renderReports(container) {
    container.innerHTML = `
      <div class="page-header">
        <h2>Financial Reports & Exports</h2>
      </div>

      <div class="content-grid">
        <!-- Holdings Report -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">Portfolio Holdings Report</div>
            <button class="btn btn-primary btn-sm" onclick="Api.download('/reports/holdings?format=csv', 'holdings_report.csv')">Export CSV</button>
          </div>
          <div class="card-body">
            <p class="text-muted mb-2">Export comprehensive position data across all your active investments, including asset class, quantities, unit prices, cost basis, and current valuations.</p>
            <div id="report-holdings-preview">
              <button class="btn btn-outline btn-sm" onclick="Pages.loadHoldingsPreview()">Preview Table</button>
            </div>
          </div>
        </div>

        <!-- Transactions Report -->
        <div class="card">
          <div class="card-header">
            <div class="card-title">Transactions Ledger</div>
            <button class="btn btn-primary btn-sm" onclick="Api.download('/reports/transactions?format=csv', 'transactions_ledger.csv')">Export CSV</button>
          </div>
          <div class="card-body">
            <p class="text-muted mb-2">Export audit-compliant transaction records covering every buy, sell, dividend, interest payout, deposit, and fee deduction with timestamps.</p>
            <div id="report-transactions-preview">
              <button class="btn btn-outline btn-sm" onclick="Pages.loadTransactionsPreview()">Preview Table</button>
            </div>
          </div>
        </div>
      </div>

      <!-- P&L Statement -->
      <div class="card">
        <div class="card-header">
          <div class="card-title">Realized & Unrealized P&L Statement</div>
          <button class="btn btn-primary btn-sm" onclick="Api.download('/reports/pnl?format=csv', 'pnl_statement.csv')">Export CSV</button>
        </div>
        <div class="card-body">
          <p class="text-muted mb-2">Review your complete capital gains and income statement, separating realized gains/losses from paper profits for tax planning and performance audits.</p>
          <div id="report-pnl-preview">
            <button class="btn btn-outline btn-sm" onclick="Pages.loadPnLPreview()">Preview Table</button>
          </div>
        </div>
      </div>
    `;
  },

  async loadHoldingsPreview() {
    const el = document.getElementById('report-holdings-preview');
    if (!el) return;
    el.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/reports/holdings?format=json');
      const list = Array.isArray(res.data) ? res.data : (res.data?.holdings || []);
      if (list.length === 0) {
        el.innerHTML = '<p class="text-muted">No holdings found.</p>';
        return;
      }
      el.innerHTML = `
        <div class="table-responsive">
          <table>
            <thead><tr><th>Symbol</th><th>Name</th><th>Category</th><th>Qty</th><th>Value</th></tr></thead>
            <tbody>
              ${list.slice(0, 5).map(h => `
                <tr>
                  <td><strong>${h.symbol || '—'}</strong></td>
                  <td>${h.name || h.instrument_name || '—'}</td>
                  <td>${h.category || h.type_code || '—'}</td>
                  <td>${h.quantity ? formatNumber(h.quantity, 2) : '—'}</td>
                  <td>${formatINR(h.current_value || h.cost || 0)}</td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
        <p class="text-muted mt-1"><small>Showing top 5 rows. Click "Export CSV" for full report.</small></p>
      `;
    } catch (err) {
      el.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async loadTransactionsPreview() {
    const el = document.getElementById('report-transactions-preview');
    if (!el) return;
    el.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/reports/transactions?format=json');
      const list = Array.isArray(res.data) ? res.data : (res.data?.transactions || []);
      if (list.length === 0) {
        el.innerHTML = '<p class="text-muted">No transactions found.</p>';
        return;
      }
      el.innerHTML = `
        <div class="table-responsive">
          <table>
            <thead><tr><th>Date</th><th>Type</th><th>Instrument</th><th>Net Amount</th></tr></thead>
            <tbody>
              ${list.slice(0, 5).map(t => `
                <tr>
                  <td>${formatDate(t.date || t.transaction_date)}</td>
                  <td>${t.type || t.transaction_type}</td>
                  <td>${t.symbol || t.name || '—'}</td>
                  <td>${formatINR(t.net_amount)}</td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
        <p class="text-muted mt-1"><small>Showing top 5 rows. Click "Export CSV" for full report.</small></p>
      `;
    } catch (err) {
      el.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  },

  async loadPnLPreview() {
    const el = document.getElementById('report-pnl-preview');
    if (!el) return;
    el.innerHTML = '<div class="spinner"></div>';
    try {
      const res = await Api.get('/reports/pnl?format=json');
      const d = res.data || {};
      const positions = Array.isArray(d.positions) ? d.positions : (Array.isArray(d.pnl_items) ? d.pnl_items : []);
      if (positions.length === 0) {
        el.innerHTML = '<p class="text-muted">No P&L data available.</p>';
        return;
      }
      el.innerHTML = `
        <div class="table-responsive">
          <table>
            <thead><tr><th>Instrument</th><th>Cost (₹)</th><th>Current Value (₹)</th><th>Unrealized P&L</th></tr></thead>
            <tbody>
              ${positions.slice(0, 5).map(p => `
                <tr>
                  <td><strong>${p.name || p.symbol}</strong></td>
                  <td>${formatINR(p.cost_basis || p.cost_cents / 100 || 0)}</td>
                  <td>${formatINR(p.current_value || p.current_value_cents / 100 || 0)}</td>
                  <td class="${(p.unrealized_gain_loss || 0) >= 0 ? 'text-success' : 'text-danger'} font-bold">
                    ${(p.unrealized_gain_loss || 0) >= 0 ? '+' : ''}${formatINR(p.unrealized_gain_loss || 0)}
                  </td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
        <p class="text-muted mt-1"><small>Showing top 5 positions. Click "Export CSV" for full report.</small></p>
      `;
    } catch (err) {
      el.innerHTML = `<div class="alert alert-error">${err.message}</div>`;
    }
  }
};

