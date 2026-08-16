const form = document.querySelector('#searchForm');
const input = document.querySelector('#searchInput');
const page = document.querySelector('#searchPage');
const resultsArea = document.querySelector('#resultsArea');

async function renderResults(query) {
  page.classList.add('has-results');

  const trimmedQuery = query.trim();
  if (!trimmedQuery) {
    resultsArea.innerHTML = '<p class="empty-state">Enter a word to search.</p>';
    return;
  }

  resultsArea.innerHTML = '<p class="result-count">Searching your crawler index…</p>';

  let results;
  try {
    const response = await fetch(
    `https://backend.nileshsahu.me/search/api/search?q=${encodeURIComponent(trimmedQuery)}`
);
    const data = await response.json();
    if (!response.ok) throw new Error(data.error || 'Search failed.');
    results = data.results;
  } catch (error) {
    resultsArea.innerHTML = `<p class="empty-state">${error.message} Start <code>searchengine.exe --server</code> and try again.</p>`;
    return;
  }

  if (results.length === 0) {
    resultsArea.innerHTML = `<p class="empty-state">No results found for “${trimmedQuery}”.</p>`;
    return;
  }

  resultsArea.replaceChildren();
  const count = document.createElement('p');
  count.className = 'result-count';
  count.textContent = `${results.length} result${results.length === 1 ? '' : 's'} for “${trimmedQuery}”`;
  resultsArea.append(count);

  results.forEach((result) => {
    const article = document.createElement('article');
    article.className = 'result';

    const url = document.createElement('span');
    url.className = 'result-url';
    url.textContent = result.url;

    const title = document.createElement('a');
    title.className = 'result-title';
    title.href = result.url;
    title.textContent = result.title || result.url;

    const description = document.createElement('p');
    description.className = 'result-description';
    description.textContent = result.description;

    article.append(url, title, description);
    resultsArea.append(article);
  });
}

form.addEventListener('submit', (event) => {
  event.preventDefault();
  renderResults(input.value);
});
