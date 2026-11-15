/**
 * InvestSphere Authentication Module
 * Handles login, registration, session restore, logout.
 * Uses a server-issued HMAC-signed Math Captcha (no third-party reCAPTCHA dependency).
 */

const Auth = {
  // Math captcha state
  captchaChallengeId: null,
  resetCaptchaChallengeId: null,

  async init() {
    this.bindEvents();
    await this.loadMathCaptcha();
    this.checkSession();
  },

  /* ── Math Captcha ─────────────────────────────────────────────── */

  async loadMathCaptcha() {
    try {
      const cfg = await Api.get('/auth/config');
      if (cfg && cfg.data) {
        this.setCaptchaUI(cfg.data.captcha_question, cfg.data.captcha_challenge_id);
      }
    } catch (e) {
      const q = document.getElementById('captcha-question-text');
      if (q) q.textContent = 'What is 7 + 5?';
    }
  },

  async refreshMathCaptcha() {
    const btn = document.getElementById('btn-refresh-captcha');
    if (btn) { btn.textContent = '…'; btn.disabled = true; }
    const answerInput = document.getElementById('reg-captcha-answer');
    if (answerInput) answerInput.value = '';
    try {
      const res = await Api.get('/auth/captcha');
      if (res && res.data) {
        this.setCaptchaUI(res.data.question, res.data.challenge_id);
      }
    } catch (e) {
      console.warn('Could not refresh captcha:', e);
    } finally {
      if (btn) { btn.textContent = '↺'; btn.disabled = false; }
    }
  },

  setCaptchaUI(question, challengeId) {
    this.captchaChallengeId = challengeId;
    const q = document.getElementById('captcha-question-text');
    if (q) q.textContent = question || 'What is 7 + 5?';
  },

  async loadResetCaptcha() {
    try {
      const res = await Api.get('/auth/captcha');
      if (res && res.data) {
        this.resetCaptchaChallengeId = res.data.challenge_id;
        const q = document.getElementById('reset-captcha-question-text');
        if (q) q.textContent = res.data.question || 'What is 8 + 4?';
      }
    } catch (e) {
      const q = document.getElementById('reset-captcha-question-text');
      if (q) q.textContent = 'What is 8 + 4?';
    }
  },

  async refreshResetCaptcha() {
    const btn = document.getElementById('btn-refresh-reset-captcha');
    if (btn) { btn.textContent = '…'; btn.disabled = true; }
    const answerInput = document.getElementById('reset-captcha-answer');
    if (answerInput) answerInput.value = '';
    try {
      const res = await Api.get('/auth/captcha');
      if (res && res.data) {
        this.resetCaptchaChallengeId = res.data.challenge_id;
        const q = document.getElementById('reset-captcha-question-text');
        if (q) q.textContent = res.data.question || 'What is 8 + 4?';
      }
    } catch (e) {
      console.warn('Could not refresh reset captcha:', e);
    } finally {
      if (btn) { btn.textContent = '↺'; btn.disabled = false; }
    }
  },

  /* ── Event Binding ───────────────────────────────────────────── */

  bindEvents() {
    // Switch between login, register, and reset password
    document.getElementById('show-register')?.addEventListener('click', (e) => {
      e.preventDefault();
      this.toggleForms('register');
    });
    document.getElementById('show-login')?.addEventListener('click', (e) => {
      e.preventDefault();
      this.toggleForms('login');
    });
    document.getElementById('show-reset')?.addEventListener('click', (e) => {
      e.preventDefault();
      this.toggleForms('reset');
    });
    document.getElementById('show-login-from-reset')?.addEventListener('click', (e) => {
      e.preventDefault();
      this.toggleForms('login');
    });

    // Submit buttons
    document.getElementById('btn-login')?.addEventListener('click', () => this.login());
    document.getElementById('btn-register')?.addEventListener('click', () => this.register());
    document.getElementById('btn-reset-password')?.addEventListener('click', () => this.resetPassword());

    // Math captcha refresh buttons
    document.getElementById('btn-refresh-captcha')?.addEventListener('click', () => this.refreshMathCaptcha());
    document.getElementById('btn-refresh-reset-captcha')?.addEventListener('click', () => this.refreshResetCaptcha());

    // Enter-key shortcuts
    document.getElementById('login-password')?.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') this.login();
    });
    document.getElementById('reg-captcha-answer')?.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') this.register();
    });
    document.getElementById('reg-confirm')?.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') this.register();
    });
    document.getElementById('reset-captcha-answer')?.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') this.resetPassword();
    });

    // Logout
    document.getElementById('btn-logout')?.addEventListener('click', () => this.logout());

    // Session expiry event
    window.addEventListener('auth:expired', () => {
      this.showAuthView();
      if (typeof showToast === 'function') {
        showToast('Your session has expired. Please sign in again.', 'warning');
      }
    });
  },

  /* ── Form Toggle ─────────────────────────────────────────────── */

  toggleForms(target) {
    const loginForm = document.getElementById('login-form');
    const registerForm = document.getElementById('register-form');
    const resetForm = document.getElementById('reset-form');

    document.getElementById('login-error')?.classList.add('hidden');
    document.getElementById('register-error')?.classList.add('hidden');
    document.getElementById('reset-error')?.classList.add('hidden');
    document.getElementById('reset-success')?.classList.add('hidden');

    if (target === 'register') {
      loginForm?.classList.add('hidden');
      resetForm?.classList.add('hidden');
      registerForm?.classList.remove('hidden');
      this.loadMathCaptcha();
      document.getElementById('reg-name')?.focus();
    } else if (target === 'reset') {
      loginForm?.classList.add('hidden');
      registerForm?.classList.add('hidden');
      resetForm?.classList.remove('hidden');
      this.loadResetCaptcha();
      document.getElementById('reset-email')?.focus();
    } else {
      registerForm?.classList.add('hidden');
      resetForm?.classList.add('hidden');
      loginForm?.classList.remove('hidden');
      document.getElementById('login-email')?.focus();
    }
  },

  /* ── Login ───────────────────────────────────────────────────── */

  async login() {
    const emailInput = document.getElementById('login-email');
    const passwordInput = document.getElementById('login-password');
    const errorBox = document.getElementById('login-error');
    const btn = document.getElementById('btn-login');

    const email = emailInput.value.trim();
    const password = passwordInput.value;

    errorBox.classList.add('hidden');

    if (!email || !password) {
      errorBox.textContent = 'Please enter both email and password.';
      errorBox.classList.remove('hidden');
      return;
    }

    btn.disabled = true;
    btn.textContent = 'Signing in…';

    try {
      const res = await Api.post('/auth/login', { email, password });
      if ((res.success || res.status === 'ok') && res.data) {
        Api.setToken(res.data.token);
        Api.setUser(res.data.user);
        this.updateUserUI(res.data.user);
        this.showAppView();
        if (typeof showToast === 'function') {
          showToast(`Welcome back, ${res.data.user.full_name || 'Investor'}!`, 'success');
        }
        if (window.App && typeof window.App.navigate === 'function') {
          window.App.navigate('dashboard');
        }
      } else {
        throw new Error(res.error || 'Invalid credentials');
      }
    } catch (err) {
      errorBox.textContent = err.message || 'Invalid email or password.';
      errorBox.classList.remove('hidden');
    } finally {
      btn.disabled = false;
      btn.textContent = 'Sign In';
    }
  },

  /* ── Reset Password ─────────────────────────────────────────── */

  async resetPassword() {
    const email = document.getElementById('reset-email')?.value.trim() || '';
    const new_password = document.getElementById('reset-password')?.value || '';
    const confirm_password = document.getElementById('reset-confirm')?.value || '';
    const captcha_answer = document.getElementById('reset-captcha-answer')?.value.trim() || '';
    const captcha_challenge_id = this.resetCaptchaChallengeId;

    const errorBox = document.getElementById('reset-error');
    const successBox = document.getElementById('reset-success');
    const btn = document.getElementById('btn-reset-password');

    errorBox?.classList.add('hidden');
    successBox?.classList.add('hidden');

    if (!email) {
      errorBox.textContent = 'Please enter your registered email address.';
      errorBox.classList.remove('hidden');
      return;
    }
    if (!new_password) {
      errorBox.textContent = 'Please enter a new password.';
      errorBox.classList.remove('hidden');
      return;
    }
    if (new_password.length < 8) {
      errorBox.textContent = 'Password must be at least 8 characters with letters and numbers.';
      errorBox.classList.remove('hidden');
      return;
    }
    if (new_password !== confirm_password) {
      errorBox.textContent = 'Passwords do not match.';
      errorBox.classList.remove('hidden');
      return;
    }
    if (!captcha_answer) {
      errorBox.textContent = 'Please solve the security challenge.';
      errorBox.classList.remove('hidden');
      return;
    }

    btn.disabled = true;
    btn.textContent = 'Resetting password…';

    try {
      const res = await Api.post('/auth/reset-password', {
        email,
        new_password,
        confirm_password,
        captcha_challenge_id,
        captcha_answer
      });

      if ((res.success || res.status === 'ok') && res.data) {
        Api.setToken(res.data.token);
        Api.setUser(res.data.user);
        this.updateUserUI(res.data.user);

        if (typeof showToast === 'function') {
          showToast('Password reset successfully! Welcome back 🎉', 'success');
        }

        this.showAppView();
        if (window.App && typeof window.App.navigate === 'function') {
          window.App.navigate('dashboard');
        }
      } else {
        throw new Error(res.error || 'Password reset failed.');
      }
    } catch (err) {
      errorBox.textContent = err.message || 'Password reset failed. Please check your inputs.';
      errorBox.classList.remove('hidden');
      this.refreshResetCaptcha();
    } finally {
      btn.disabled = false;
      btn.textContent = 'Reset Password & Sign In';
    }
  },

  /* ── Register ────────────────────────────────────────────────── */

  async register() {
    const full_name     = document.getElementById('reg-name')?.value.trim() || '';
    const email         = document.getElementById('reg-email')?.value.trim() || '';
    const mobile_number = document.getElementById('reg-mobile')?.value.trim() || '';
    const date_of_birth = document.getElementById('reg-dob')?.value.trim() || '';
    const country       = document.getElementById('reg-country')?.value || 'India';
    const state         = document.getElementById('reg-state')?.value || 'Maharashtra';
    const pan           = document.getElementById('reg-pan')?.value.trim().toUpperCase() || '';
    const password      = document.getElementById('reg-password')?.value || '';
    const confirm_password = document.getElementById('reg-confirm')?.value || '';
    const captcha_answer   = document.getElementById('reg-captcha-answer')?.value.trim() || '';
    const captcha_challenge_id = this.captchaChallengeId || '';

    const errorBox  = document.getElementById('register-error');
    const successBox = document.getElementById('register-success');
    const btn       = document.getElementById('btn-register');

    errorBox.classList.add('hidden');
    successBox?.classList.add('hidden');

    // ── Client-side validation ──────────────────────────────────
    if (!full_name || !email || !password || !confirm_password) {
      errorBox.textContent = 'Please fill in all required fields (Name, Email, Password).';
      errorBox.classList.remove('hidden');
      return;
    }

    const normalizedMobile = mobile_number.replace(/^(\+91|91)/, '').replace(/[\s\-\(\)]/g, '');
    if (mobile_number && !/^[6-9]\d{9}$/.test(normalizedMobile)) {
      errorBox.textContent = 'Please enter a valid 10-digit Indian mobile number (e.g. 9876543210).';
      errorBox.classList.remove('hidden');
      return;
    }

    if (pan && !/^[A-Z]{5}[0-9]{4}[A-Z]$/.test(pan)) {
      errorBox.textContent = 'Invalid PAN format. Expected: 5 letters, 4 digits, 1 letter (e.g. ABCDE1234F).';
      errorBox.classList.remove('hidden');
      return;
    }

    if (password !== confirm_password) {
      errorBox.textContent = 'Passwords do not match. Please re-enter.';
      errorBox.classList.remove('hidden');
      return;
    }

    if (password.length < 8) {
      errorBox.textContent = 'Password must be at least 8 characters with letters and numbers.';
      errorBox.classList.remove('hidden');
      return;
    }

    if (!captcha_answer) {
      errorBox.textContent = 'Please solve the security question before registering.';
      errorBox.classList.remove('hidden');
      document.getElementById('reg-captcha-answer')?.focus();
      return;
    }

    btn.disabled = true;
    btn.textContent = 'Creating account…';

    try {
      const res = await Api.post('/auth/register', {
        full_name,
        email,
        mobile_number,
        date_of_birth,
        country,
        state,
        pan,
        password,
        confirm_password,
        captcha_challenge_id,
        captcha_answer
      });

      if ((res.success || res.status === 'ok') && res.data) {
        Api.setToken(res.data.token);
        Api.setUser(res.data.user);
        this.updateUserUI(res.data.user);

        if (typeof showToast === 'function') {
          showToast('Account created! Welcome to InvestSphere 🎉', 'success');
        }

        this.showAppView();
        if (window.App && typeof window.App.navigate === 'function') {
          window.App.navigate('dashboard');
        }
      } else {
        throw new Error(res.error || 'Registration failed. Please check your inputs.');
      }
    } catch (err) {
      errorBox.textContent = err.message || 'Registration failed. Please check your inputs.';
      errorBox.classList.remove('hidden');
      // Refresh captcha after failure so the old token cannot be reused
      this.refreshMathCaptcha();
    } finally {
      btn.disabled = false;
      btn.textContent = 'Register';
    }
  },

  /* ── Logout ──────────────────────────────────────────────────── */

  async logout() {
    try {
      await Api.post('/auth/logout', {});
    } catch {
      // Ignore network errors during logout
    } finally {
      Api.clearSession();
      this.showAuthView();
      if (typeof showToast === 'function') {
        showToast('Signed out successfully.', 'info');
      }
    }
  },

  /* ── Session Restore ─────────────────────────────────────────── */

  async checkSession() {
    const token = Api.getToken();
    if (!token) {
      this.showAuthView();
      return;
    }

    try {
      const res = await Api.get('/auth/me');
      if ((res.success || res.status === 'ok') && res.data) {
        Api.setUser(res.data);
        this.updateUserUI(res.data);
        this.showAppView();
        if (window.App && typeof window.App.navigate === 'function') {
          window.App.navigate('dashboard');
        }
      } else {
        this.showAuthView();
      }
    } catch {
      Api.clearSession();
      this.showAuthView();
    }
  },

  /* ── UI Helpers ──────────────────────────────────────────────── */

  updateUserUI(user) {
    const nameEl   = document.getElementById('sidebar-user-name');
    const emailEl  = document.getElementById('sidebar-user-email');
    const avatarEl = document.getElementById('user-avatar');

    if (nameEl)   nameEl.textContent  = user.full_name || 'User';
    if (emailEl)  emailEl.textContent = user.email || '';
    if (avatarEl) {
      avatarEl.textContent = (user.full_name || user.email || 'U')[0].toUpperCase();
    }
  },

  showAuthView() {
    document.getElementById('auth-section')?.classList.remove('hidden');
    document.getElementById('app-section')?.classList.add('hidden');
  },

  showAppView() {
    document.getElementById('auth-section')?.classList.add('hidden');
    document.getElementById('app-section')?.classList.remove('hidden');
  }
};
