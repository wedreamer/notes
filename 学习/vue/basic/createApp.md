# createApp 做了什么

## 官方文档

- [官方文档](https://cn.vuejs.org/guide/essentials/application.html)

## 相关代码

```html
<div id="app">
  <button @click="count++">{{ count }}</button>
</div>
```

```ts
import { createApp } from 'vue'

const app = createApp({
  data() {
    return {
      count: 0
    }
  }
})

app.mount('#app')
```

## 一些问题

- createApp 都有什么行为


