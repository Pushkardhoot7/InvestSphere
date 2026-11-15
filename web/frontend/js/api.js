/**
 * InvestSphere API Client
 * Manages HTTP communication with the REST API backend.
 */

const API_BASE = window.INVESTSPHERE_API || '/api';

const Api = {
  getToken() {
    return localStorage.getItem('investsphere_token');
  },

  setToken(token) {
    localStorage.setItem('investsphere_token', token);
  },

  getUser() {
    try {
      const u = localStorage.getItem('investsphere_user');
      return u ? JSON.parse(u) : null;
    } catch {
      return null;
    }
  },

  setUser(user) {
    localStorage.setItem('investsphere_user', JSON.stringify(user));
  },

  clearSession() {
    localStorage.removeItem('investsphere_token');
    localStorage.removeItem('investsphere_user');
  },

  async request(endpoint, options = {}) {
    const url = endpoint.startsWith('http') ? endpoint : `${API_BASE}${endpoint}`;
    const headers = {
      'Accept': 'application/json',
      ...(options.headers || {})
    };

    const token = this.getToken();
    if (token) {
      headers['Authorization'] = `Bearer ${token}`;
    }

    if (options.body && typeof options.body === 'object' && !(options.body instanceof FormData)) {
      headers['Content-Type'] = 'application/json';
      options.body = JSON.stringify(options.body);
    }

    try {
      const response = await fetch(url, { ...options, headers });

      const isAuthRoute = endpoint.includes('/auth/login') || 
                          endpoint.includes('/auth/register') || 
                          endpoint.includes('/auth/reset-password') ||
                          endpoint.includes('/auth/config') ||
                          endpoint.includes('/auth/captcha');

      if (response.status === 401 && !isAuthRoute) {
        // Token expired or invalid for a protected route
        this.clearSession();
        // Only show the session-expired toast if the app section is actually visible
        // (i.e., user was logged in and their session just expired mid-use).
        // Don't show it during the initial checkSession() call on the sign-in page.
        const appVisible = !document.getElementById('app-section')?.classList.contains('hidden');
        if (appVisible) {
          window.dispatchEvent(new CustomEvent('auth:expired'));
        }
        throw new Error('Session expired. Please sign in again.');
      }

      // Check if CSV or binary response
      const contentType = response.headers.get('content-type') || '';
      if (contentType.includes('text/csv') || contentType.includes('application/octet-stream')) {
        if (!response.ok) {
          throw new Error('Failed to download file');
        }
        return await response.blob();
      }

      const data = await response.json().catch(() => ({}));

      if (!response.ok) {
        const errorMsg = data.error || (data.errors ? Object.values(data.errors).join(', ') : `HTTP ${response.status}`);
        throw new Error(errorMsg);
      }

      return data;
    } catch (err) {
      console.error(`API Error [${options.method || 'GET'} ${endpoint}]:`, err);
      throw err;
    }
  },

  get(endpoint, params = null) {
    let url = endpoint;
    if (params) {
      const qs = new URLSearchParams(
        Object.entries(params).filter(([_, v]) => v !== null && v !== undefined && v !== '')
      ).toString();
      if (qs) url += (url.includes('?') ? '&' : '?') + qs;
    }
    return this.request(url, { method: 'GET' });
  },

  post(endpoint, data) {
    return this.request(endpoint, {
      method: 'POST',
      body: data
    });
  },

  put(endpoint, data) {
    return this.request(endpoint, {
      method: 'PUT',
      body: data
    });
  },

  patch(endpoint, data) {
    return this.request(endpoint, {
      method: 'PATCH',
      body: data
    });
  },

  delete(endpoint) {
    return this.request(endpoint, { method: 'DELETE' });
  },

  async download(endpoint, defaultFilename = 'download.csv') {
    const blob = await this.request(endpoint, { method: 'GET' });
    const downloadUrl = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = downloadUrl;
    a.download = defaultFilename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    window.URL.revokeObjectURL(downloadUrl);
  }
};

