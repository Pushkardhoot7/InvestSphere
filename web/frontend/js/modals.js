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
  },

  open(modalId) {
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
      this.populateInvestmentDropdown();
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
      const fdFields = document.getElementById('fd-fields');
      const qtyGroup = document.getElementById('inv-qty-group');
      const priceRow = document.getElementById('inv-price-row');

      if (isFD) {
        fdFields?.classList.remove('hidden');
        qtyGroup?.classList.add('hidden');
        priceRow?.classList.add('hidden');
      } else {
        fdFields?.classList.add('hidden');
        qtyGroup?.classList.remove('hidden');
        priceRow?.classList.remove('hidden');
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

  async populateInvestmentDropdown() {
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
    }

    btn.disabled = true;
    btn.textContent = 'Adding…';

    try {
      const res = await Api.post('/investments', payload);
      if (res.status === 'ok') {
        showToast(`Investment added: ${name}`, 'success');
        this.close('modal-add-investment');
        // Reset form
        document.getElementById('inv-symbol').value = '';
        document.getElementById('inv-name').value = '';
        document.getElementById('inv-qty').value = '';
        document.getElementById('inv-price').value = '';
        document.getElementById('inv-total').value = '';
        document.getElementById('inv-notes').value = '';
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

      if (res.status === 'ok') {
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

