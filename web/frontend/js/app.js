/**
 * InvestSphere Application Controller
 * Handles navigation routing, mobile sidebar toggle, and global initialization.
 */

const App = {
  currentPage: 'dashboard',

  pageTitles: {
    dashboard: 'Dashboard',
    portfolio: 'Portfolio Management',
    transactions: 'Transactions Ledger',
    stocks: 'Stocks & Live Market',
    market: 'Stocks & Live Market',
    ipo: 'IPO Center',
    news: 'Market News & Insights',
    'market-news': 'Market News & Insights',
    'precious-metals': 'Gold & Silver Rates (India / Maharashtra)',
    sip: 'SIP & Mutual Funds',
    swp: 'SWP Calculator',
    maturity: 'Maturity Tracker',
    analytics: 'Portfolio Analytics',
    reports: 'Financial Reports'
  },

  init() {
    this.bindNavigation();
    this.bindSidebarToggle();

    // Initialize sub-modules
    if (typeof Modals !== 'undefined' && Modals.init) {
      Modals.init();
    }
    if (typeof Auth !== 'undefined' && Auth.init) {
      Auth.init();
    }

    // Handle browser back/forward buttons
    window.addEventListener('popstate', (e) => {
      const page = e.state?.page || 'dashboard';
      this.navigate(page, false);
    });
  },

  bindNavigation() {
    document.querySelectorAll('.sidebar-nav .nav-item').forEach((link) => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const page = link.getAttribute('data-page');
        if (page) {
          this.navigate(page);
          this.closeSidebar();
        }
      });
    });
  },

  bindSidebarToggle() {
    const toggleBtn = document.getElementById('sidebar-toggle');
    const closeBtn = document.getElementById('sidebar-close');
    const overlay = document.getElementById('sidebar-overlay');
    const sidebar = document.getElementById('sidebar');

    if (toggleBtn) {
      toggleBtn.addEventListener('click', () => {
        sidebar?.classList.add('open');
        overlay?.classList.add('show');
      });
    }

    if (closeBtn) {
      closeBtn.addEventListener('click', () => this.closeSidebar());
    }

    if (overlay) {
      overlay.addEventListener('click', () => this.closeSidebar());
    }
  },

  closeSidebar() {
    const sidebar = document.getElementById('sidebar');
    const overlay = document.getElementById('sidebar-overlay');
    sidebar?.classList.remove('open');
    overlay?.classList.remove('show');
  },

  navigate(page, push = true) {
    if (!this.pageTitles[page]) {
      page = 'dashboard';
    }

    this.currentPage = page;

    // Update active nav item
    document.querySelectorAll('.sidebar-nav .nav-item').forEach((link) => {
      const p = link.getAttribute('data-page');
      if (p === page || (page === 'stocks' && p === 'market') || (page === 'news' && p === 'market-news')) {
        link.classList.add('active');
      } else {
        link.classList.remove('active');
      }
    });

    // Update topbar title
    const titleEl = document.getElementById('page-title');
    if (titleEl) {
      titleEl.textContent = this.pageTitles[page];
    }

    // Push to browser history
    if (push) {
      window.history.pushState({ page }, this.pageTitles[page], `#${page}`);
    }

    // Render page content
    const container = document.getElementById('page-content');
    if (!container) return;

    switch (page) {
      case 'dashboard':
        Pages.renderDashboard(container);
        break;
      case 'portfolio':
        Pages.renderPortfolio(container);
        break;
      case 'transactions':
        Pages.renderTransactions(container);
        break;
      case 'stocks':
      case 'market':
        Pages.renderStocks(container);
        break;
      case 'ipo':
        Pages.renderIPO(container);
        break;
      case 'news':
      case 'market-news':
        Pages.renderNews(container);
        break;
      case 'precious-metals':
        Pages.renderPreciousMetals(container);
        break;
      case 'sip':
        Pages.renderSIPAndMutualFunds(container);
        break;
      case 'swp':
        Pages.renderSWP(container);
        break;
      case 'maturity':
        Pages.renderMaturity(container);
        break;
      case 'analytics':
        Pages.renderAnalytics(container);
        break;
      case 'reports':
        Pages.renderReports(container);
        break;
      default:
        Pages.renderDashboard(container);
    }

    window.scrollTo({ top: 0, behavior: 'smooth' });
  },

  refreshCurrentPage() {
    this.navigate(this.currentPage, false);
  }
};

// Global startup when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
  window.App = App;
  App.init();

  // If hash present in URL, navigate to it after auth check
  const hash = window.location.hash.replace('#', '');
  if (hash && App.pageTitles[hash]) {
    App.currentPage = hash;
  }
});
