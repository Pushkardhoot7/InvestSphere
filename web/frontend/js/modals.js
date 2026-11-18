/**
 * InvestSphere Modals & Notifications
 * Manages modal dialogs and toast alerts.
 */

function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  if (!container) return;

  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  const icon = type === 'success' ? '✓' : type === 'error' ? '✕' : type === 'warning' ? '⚠' : 'ℹ';
  toast.innerHTML = `<span>${icon}</span> <span>${message}</span>`;

  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateX(50px)';
    toast.style.transition = 'all 0.3s ease';
    setTimeout(() => toast.remove(), 300);
  }, 3500);
}

const Modals = {
  init() {
    this.bindEvents();
    this.setupInvestmentCategoryChange();
    this.setupAutoCalculations();
    this.setupStockAutocomplete();
  },

  openAddInvestmentForStock(symbol, name, price) {
    this.open('modal-add-investment');
    const catSelect = document.getElementById('inv-category');
    if (catSelect) catSelect.value = 'STOCK';
    document.getElementById('inv-stock-search-group')?.classList.remove('hidden');
    document.getElementById('fd-fields')?.classList.add('hidden');
    document.getElementById('inv-qty-group')?.classList.remove('hidden');
    document.getElementById('inv-price-row')?.classList.remove('hidden');

    document.getElementById('inv-symbol').value = symbol || '';
    document.getElementById('inv-name').value = name || symbol || '';
    if (price && price > 0) {
      document.getElementById('inv-price').value = price;
    }
    const searchInput = document.getElementById('inv-stock-search');
    if (searchInput) searchInput.value = `${symbol} — ${name}`;

    const today = new Date().toISOString().split('T')[0];
    const dateInput = document.getElementById('inv-date');
    if (dateInput && !dateInput.value) dateInput.value = today;

    setTimeout(() => {
      document.getElementById('inv-qty')?.focus();
    }, 150);
  },

  open(modalId, selectedInvId = null) {
    const el = document.getElementById(modalId);
    if (!el) return;
    el.classList.remove('hidden');

    if (modalId === 'modal-add-investment') {
      const today = new Date().toISOString().split('T')[0];
      const dateInput = document.getElementById('inv-date');
      if (dateInput && !dateInput.value) dateInput.value = today;
      document.getElementById('add-inv-error')?.classList.add('hidden');
    } else if (modalId === 'modal-add-transaction') {
      const today = new Date().toISOString().split('T')[0];
      const dateInput = document.getElementById('tx-date');
      if (dateInput && !dateInput.value) dateInput.value = today;
      document.getElementById('add-tx-error')?.classList.add('hidden');
      this.populateInvestmentDropdown(selectedInvId);
    }
  },

  close(modalId) {
    const el = document.getElementById(modalId);
    if (el) el.classList.add('hidden');
  },

  bindEvents() {
    // Close buttons with data-close
    document.querySelectorAll('[data-close]').forEach((btn) => {
      btn.addEventListener('click', (e) => {
        const targetId = btn.getAttribute('data-close');
        this.close(targetId);
      });
    });

    // Close on backdrop click
    document.querySelectorAll('.modal-backdrop').forEach((backdrop) => {
      backdrop.addEventListener('click', (e) => {
        if (e.target === backdrop) {
          backdrop.classList.add('hidden');
        }
      });
    });

    // Close on ESC
    document.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') {
        document.querySelectorAll('.modal-backdrop:not(.hidden)').forEach((b) => b.classList.add('hidden'));
      }
    });

    // Topbar Add Investment button
    const btnAddInv = document.getElementById('btn-add-investment');
    if (btnAddInv) {
      btnAddInv.addEventListener('click', () => this.open('modal-add-investment'));
    }

    // Modal submit buttons
    const btnSaveInv = document.getElementById('btn-save-investment');
    if (btnSaveInv) {
      btnSaveInv.addEventListener('click', () => this.saveInvestment());
    }

    const btnSaveTx = document.getElementById('btn-save-transaction');
    if (btnSaveTx) {
      btnSaveTx.addEventListener('click', () => this.saveTransaction());
    }
  },

  setupInvestmentCategoryChange() {
    const catSelect = document.getElementById('inv-category');
    if (!catSelect) return;

    catSelect.addEventListener('change', () => {
      const cat = catSelect.value;
      const isFD = cat === 'FIXED_DEPOSIT';
      const isBond = cat === 'BOND';
      const isStock = cat === 'STOCK';
      const fdFields = document.getElementById('fd-fields');
      const bondFields = document.getElementById('bond-fields');
      const qtyGroup = document.getElementById('inv-qty-group');
      const priceRow = document.getElementById('inv-price-row');
      const stockSearchGroup = document.getElementById('inv-stock-search-group');

      if (stockSearchGroup) {
        stockSearchGroup.classList.toggle('hidden', !isStock);
      }

      if (isFD) {
        fdFields?.classList.remove('hidden');
        bondFields?.classList.add('hidden');
        qtyGroup?.classList.add('hidden');
        priceRow?.classList.add('hidden');
      } else if (isBond) {
        fdFields?.classList.add('hidden');
        bondFields?.classList.remove('hidden');
        qtyGroup?.classList.remove('hidden');
        priceRow?.classList.remove('hidden');
        const symEl = document.getElementById('inv-symbol');
        const nameEl = document.getElementById('inv-name');
        if (symEl && !symEl.value) symEl.value = 'GOI-7.18-2033';
        if (nameEl && !nameEl.value) nameEl.value = '7.18% Government of India Sovereign Bond 2033';
        const faceEl = document.getElementById('inv-bond-face');
        const couponEl = document.getElementById('inv-bond-coupon');
        if (faceEl && !faceEl.value) faceEl.value = '1000';
        if (couponEl && !couponEl.value) couponEl.value = '7.18';
      } else {
        fdFields?.classList.add('hidden');
        bondFields?.classList.add('hidden');
        qtyGroup?.classList.remove('hidden');
        priceRow?.classList.remove('hidden');
      }

      // Auto-fill defaults for gold
      if (cat === 'GOLD') {
        const symEl = document.getElementById('inv-symbol');
        const nameEl = document.getElementById('inv-name');
        if (symEl && !symEl.value) symEl.value = 'GOLD-24K';
        if (nameEl && !nameEl.value) nameEl.value = '24K Fine Gold (per gram)';
      }
    });
  },

  setupStockAutocomplete() {
    const searchInput = document.getElementById('inv-stock-search');
    const suggBox = document.getElementById('inv-stock-suggestions');
    if (!searchInput || !suggBox) return;

    let debounceTimer = null;

    searchInput.addEventListener('input', () => {
      clearTimeout(debounceTimer);
      const q = searchInput.value.trim();
      if (q.length < 1) {
        suggBox.classList.add('hidden');
        suggBox.innerHTML = '';
        return;
      }

      debounceTimer = setTimeout(async () => {
        try {
          const res = await Api.get(`/market/search?q=${encodeURIComponent(q)}`);
          const matches = res.data || [];
          if (matches.length === 0) {
            suggBox.innerHTML = `<div style="padding:0.6rem 0.8rem; font-size:0.85rem; color:#64748b;">No stocks found for "${q}"</div>`;
            suggBox.classList.remove('hidden');
            return;
          }

          suggBox.innerHTML = matches.slice(0, 8).map(st => `
            <div class="stock-sugg-item" style="padding:0.6rem 0.8rem; cursor:pointer; border-bottom:1px solid #f1f5f9; display:flex; justify-content:space-between; align-items:center;"
                 data-sym="${st.display_symbol || st.symbol}"
                 data-name="${st.name}"
                 data-price="${st.price || 0}">
              <div>
                <strong style="color:var(--text);">${st.display_symbol || st.symbol}</strong>
                <span style="font-size:0.8rem; color:#64748b; margin-left:0.4rem;">${st.name}</span>
                <span style="font-size:0.75rem; background:#f1f5f9; padding:2px 6px; border-radius:4px; margin-left:0.3rem;">${st.exchange || 'NSE'}</span>
              </div>
              <div style="font-weight:600; color:var(--primary); font-size:0.9rem;">
                ₹${Number(st.price || 0).toLocaleString('en-IN', {minimumFractionDigits: 2, maximumFractionDigits: 2})}
              </div>
            </div>
          `).join('');

          suggBox.querySelectorAll('.stock-sugg-item').forEach(item => {
            item.addEventListener('mouseenter', () => item.style.background = '#f8fafc');
            item.addEventListener('mouseleave', () => item.style.background = '#ffffff');
            item.addEventListener('click', () => {
              const sym = item.getAttribute('data-sym');
              const name = item.getAttribute('data-name');
              const price = parseFloat(item.getAttribute('data-price') || 0);

              document.getElementById('inv-symbol').value = sym;
              document.getElementById('inv-name').value = name;
              if (price > 0) {
                document.getElementById('inv-price').value = price;
              }
              const today = new Date().toISOString().split('T')[0];
              const dateInput = document.getElementById('inv-date');
              if (dateInput && !dateInput.value) dateInput.value = today;

              searchInput.value = `${sym} — ${name}`;
              suggBox.classList.add('hidden');

              // Focus quantity input for user convenience
              document.getElementById('inv-qty')?.focus();
            });
          });

          suggBox.classList.remove('hidden');
        } catch (e) {
          console.warn('Stock search error:', e);
        }
      }, 200);
    });

    document.addEventListener('click', (e) => {
      if (!searchInput.contains(e.target) && !suggBox.contains(e.target)) {
        suggBox.classList.add('hidden');
      }
    });
  },

  setupAutoCalculations() {
    // Investment total calculation: qty * price
    const qtyInput = document.getElementById('inv-qty');
    const priceInput = document.getElementById('inv-price');
    const totalInput = document.getElementById('inv-total');

    const calcInvTotal = () => {
      const q = parseFloat(qtyInput?.value || 0);
      const p = parseFloat(priceInput?.value || 0);
      if (totalInput && !isNaN(q) && !isNaN(p) && q > 0 && p > 0) {
        totalInput.value = (q * p).toFixed(2);
      }
    };

    qtyInput?.addEventListener('input', calcInvTotal);
    priceInput?.addEventListener('input', calcInvTotal);

    // Transaction net amount calculation:
    // BUY: Gross + Fees + Taxes
    // SELL/DIVIDEND/INTEREST: Gross - Fees - Taxes
    const txTypeSelect = document.getElementById('tx-type');
    const txGross = document.getElementById('tx-gross');
    const txFees = document.getElementById('tx-fees');
    const txTaxes = document.getElementById('tx-taxes');
    const txNet = document.getElementById('tx-net');

    const calcTxNet = () => {
      const gross = parseFloat(txGross?.value || 0);
      const fees = parseFloat(txFees?.value || 0);
      const taxes = parseFloat(txTaxes?.value || 0);
      const type = txTypeSelect?.value;

      if (isNaN(gross) || gross <= 0) return;

      let net = gross;
      if (type === 'BUY' || type === 'DEPOSIT') {
        net = gross + fees + taxes;
      } else {
        net = Math.max(0, gross - fees - taxes);
      }
      if (txNet) txNet.value = net.toFixed(2);
    };

    txGross?.addEventListener('input', calcTxNet);
    txFees?.addEventListener('input', calcTxNet);
    txTaxes?.addEventListener('input', calcTxNet);
    txTypeSelect?.addEventListener('change', calcTxNet);
  },

  async populateInvestmentDropdown(selectedId = null) {
    const sel = document.getElementById('tx-investment');
    if (!sel) return;
    sel.innerHTML = '<option value="">Loading instruments…</option>';

    try {
      const res = await Api.get('/investments');
      const list = res.data || [];
      if (list.length === 0) {
        sel.innerHTML = '<option value="">No investments found — add one first</option>';
        return;
      }
      sel.innerHTML = list
        .map((inv) => `<option value="${inv.investment_id}">${inv.name} (${inv.symbol || inv.type_code})</option>`)
        .join('');
      // Set the pre-selected investment after the list is populated
      if (selectedId) {
        sel.value = String(selectedId);
      }
    } catch (err) {
      sel.innerHTML = '<option value="">Error loading investments</option>';
    }
  },

  async saveInvestment() {
    const errBox = document.getElementById('add-inv-error');
    const btn = document.getElementById('btn-save-investment');
    errBox.classList.add('hidden');

    const category = document.getElementById('inv-category').value;
    const symbol = document.getElementById('inv-symbol').value.trim();
    const name = document.getElementById('inv-name').value.trim();
    const purchase_date = document.getElementById('inv-date').value;
    const portfolio_tag = document.getElementById('inv-tag').value.trim() || 'DEFAULT';
    const notes = document.getElementById('inv-notes').value.trim();

    if (!symbol || !name || !purchase_date) {
      errBox.textContent = 'Please fill in symbol, name, and purchase date.';
      errBox.classList.remove('hidden');
      return;
    }

    const payload = {
      category,
      category_code: category,
      symbol,
      name,
      instrument_name: name,
      purchase_date,
      portfolio_tag,
      notes
    };

    if (category === 'FIXED_DEPOSIT') {
      const institution = document.getElementById('inv-institution').value.trim();
      const principal = parseFloat(document.getElementById('inv-principal').value || 0);
      const interest_rate = parseFloat(document.getElementById('inv-rate').value || 0);
      const maturity_date = document.getElementById('inv-maturity').value;
      const compounding_frequency = document.getElementById('inv-compounding').value;
      const payout_type = document.getElementById('inv-payout').value;

      if (!institution || principal <= 0 || !maturity_date) {
        errBox.textContent = 'Please enter institution, principal amount, and maturity date.';
        errBox.classList.remove('hidden');
        return;
      }

      payload.institution = institution;
      payload.principal_amount = principal;
      payload.interest_rate = interest_rate;
      payload.maturity_date = maturity_date;
      payload.compounding_frequency = compounding_frequency;
      payload.payout_type = payout_type;
    } else {
      const quantity = parseFloat(document.getElementById('inv-qty').value || 0);
      const unit_price = parseFloat(document.getElementById('inv-price').value || 0);
      const total_amount = parseFloat(document.getElementById('inv-total').value || 0) || (quantity * unit_price);

      if (quantity <= 0 || unit_price <= 0) {
        errBox.textContent = 'Quantity and unit price must be greater than zero.';
        errBox.classList.remove('hidden');
        return;
      }

      payload.quantity = quantity;
      payload.unit_price = unit_price;
      payload.total_amount = total_amount;
      // For Bonds, capture face value, coupon rate, and maturity
      if (category === 'BOND') {
        const bondFace = parseFloat(document.getElementById('inv-bond-face')?.value || 0) || unit_price;
        const bondCoupon = parseFloat(document.getElementById('inv-bond-coupon')?.value || 0);
        const bondFreq = document.getElementById('inv-bond-freq')?.value || 'SEMI_ANNUALLY';
        const bondMaturity = document.getElementById('inv-bond-maturity')?.value || '';
        payload.face_value = bondFace;
        payload.coupon_rate = bondCoupon;
        payload.coupon_frequency = bondFreq;
        if (bondMaturity) payload.maturity_date = bondMaturity;
      }
    }

    btn.disabled = true;
    btn.textContent = 'Adding…';

    try {
      const res = await Api.post('/investments', payload);
      if (res.success || res.status === 'ok') {
        showToast(`Investment added: ${name}`, 'success');
        this.close('modal-add-investment');
        // Reset form
        document.getElementById('inv-symbol').value = '';
        document.getElementById('inv-name').value = '';
        document.getElementById('inv-qty').value = '';
        document.getElementById('inv-price').value = '';
        document.getElementById('inv-total').value = '';
        document.getElementById('inv-notes').value = '';
        const searchInput = document.getElementById('inv-stock-search');
        if (searchInput) searchInput.value = '';
        if (category === 'FIXED_DEPOSIT') {
          document.getElementById('inv-institution').value = '';
          document.getElementById('inv-principal').value = '';
          document.getElementById('inv-rate').value = '';
          document.getElementById('inv-maturity').value = '';
        }
        // Refresh active page
        if (window.App && typeof window.App.refreshCurrentPage === 'function') {
          window.App.refreshCurrentPage();
        }
      } else {
        throw new Error(res.error || 'Failed to add investment');
      }
    } catch (err) {
      errBox.textContent = err.message || 'Error saving investment';
      errBox.classList.remove('hidden');
    } finally {
      btn.disabled = false;
      btn.textContent = 'Add Investment';
    }
  },

  async saveTransaction() {
    const errBox = document.getElementById('add-tx-error');
    const btn = document.getElementById('btn-save-transaction');
    errBox.classList.add('hidden');

    const investment_id = parseInt(document.getElementById('tx-investment').value, 10);
    const tx_type = document.getElementById('tx-type').value;
    const tx_date = document.getElementById('tx-date').value;
    const quantity = parseFloat(document.getElementById('tx-qty').value || 0);
    const gross_amount = parseFloat(document.getElementById('tx-gross').value || 0);
    const fees = parseFloat(document.getElementById('tx-fees').value || 0);
    const taxes = parseFloat(document.getElementById('tx-taxes').value || 0);
    const notes = document.getElementById('tx-notes').value.trim();

    if (!investment_id || isNaN(investment_id)) {
      errBox.textContent = 'Please select an investment.';
      errBox.classList.remove('hidden');
      return;
    }

    if (!tx_date || isNaN(gross_amount) || gross_amount <= 0) {
      errBox.textContent = 'Please enter a valid date and gross amount.';
      errBox.classList.remove('hidden');
      return;
    }

    btn.disabled = true;
    btn.textContent = 'Recording…';

    try {
      const res = await Api.post('/transactions', {
        investment_id,
        tx_type,
        transaction_type: tx_type,
        tx_date,
        transaction_date: tx_date,
        quantity,
        gross_amount,
        fees,
        taxes,
        net_amount: parseFloat(document.getElementById('tx-net')?.value || gross_amount),
        notes
      });

      if (res.success || res.status === 'ok') {
        showToast('Transaction recorded successfully', 'success');
        this.close('modal-add-transaction');
        // Reset form
        document.getElementById('tx-gross').value = '';
        document.getElementById('tx-qty').value = '';
        document.getElementById('tx-fees').value = '0';
        document.getElementById('tx-taxes').value = '0';
        document.getElementById('tx-net').value = '';
        document.getElementById('tx-notes').value = '';
        // Refresh active page
        if (window.App && typeof window.App.refreshCurrentPage === 'function') {
          window.App.refreshCurrentPage();
        }
      } else {
        throw new Error(res.error || 'Failed to record transaction');
      }
    } catch (err) {
      errBox.textContent = err.message || 'Error recording transaction';
      errBox.classList.remove('hidden');
    } finally {
      btn.disabled = false;
      btn.textContent = 'Record Transaction';
    }
  }
};

