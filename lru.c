#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
   int page_number;
   struct node *next;
   struct node *prev;
}Node;

typedef struct queue
{
   Node *front_ptr;
   Node *rear_ptr;
   int maxsize;
   int cursize;
   Node *(*get_new_node_func)(int);
   void (*enqueue_func)(struct queue*, int);
   int (*dequeue_func)(struct queue*);
   void (*print_queue_func)(struct queue*);
}Queue;

typedef struct lru_cache
{
   int maxsize;
   int cursize;
   Queue *queue_ptr;
   Node **table;
   void (*init_cache_func)(struct lru_cache *);
   void (*refer_page_func)(struct lru_cache *,int);
}Lru_cache;

Node *get_new_node(int mpage_num)
{
   Node *t = (Node *)malloc(sizeof(Node));
   t->page_number = mpage_num;
   t->next = NULL;
   t->prev = NULL;
   return t;
}

void enqueue(Queue *this, int mpage_num)
{

  Node *new_node = this->get_new_node_func(mpage_num);
  if (!this->front_ptr)
     this->front_ptr = this->rear_ptr = new_node;
  else
  {
     new_node->prev = this->rear_ptr;
     this->rear_ptr->next = new_node;
     this->rear_ptr = new_node;
  }
  this->cursize++;
}

int dequeue(Queue *this)
{
   if (this->cursize < this->maxsize)
   {
      int res = -1;
      Node *temp = this->front_ptr;
      res = temp->page_number;
      this->front_ptr = temp->next;
      this->front_ptr->prev = NULL;
      free(temp);
      this->cursize--;
      return res;
   }
   else
      return -1;
}

Node * get_front_node_addr(Queue *this)
{
   return this->front_ptr;
}

void print_queue(Queue *this)
{
   Node *start = this->front_ptr;
   while(start)
   {
      fprintf(stdout,"%d ", start->page_number);
      start = start->next;
   }
   fprintf(stdout,"\n");
}

void init_cache(Lru_cache *this)
{
   this->table = (Node **) malloc(sizeof(Node *) * this->maxsize);
   for(int i=0; i<this->maxsize; i++)
      this->table[i] = NULL;
}

void refer_page(Lru_cache *this, int mpage_num)
{
   if (mpage_num >= this->maxsize)
   {
      fprintf(stdout, "Page num is out of the scope. Allowed pages are from 0 to %d\n", this->maxsize);
      return;
   }

   /* if page not found */
   if (!this->table[mpage_num])
   {
      /* there is a space available */
      if (this->cursize < this->maxsize)
      {
         this->queue_ptr->enqueue_func(this->queue_ptr, mpage_num);
         this->table[mpage_num] = this->queue_ptr->rear_ptr;
      }
      else
      {
         this->queue_ptr->dequeue_func(this->queue_ptr);
         this->queue_ptr->enqueue_func(this->queue_ptr, mpage_num);
         this->table[mpage_num] = this->queue_ptr->rear_ptr;
      }
   }
   else
   {
      Node *temp = this->table[mpage_num];
      if (temp == this->queue_ptr->front_ptr)
      {
         this->queue_ptr->front_ptr = this->queue_ptr->front_ptr->next;
         this->queue_ptr->front_ptr->prev = NULL;

         temp->next = NULL;
         this->queue_ptr->rear_ptr->next = temp;
         temp->prev = this->queue_ptr->rear_ptr;
         this->queue_ptr->rear_ptr = temp;
      }
      else if ( temp == this->queue_ptr->rear_ptr)
      {
         /* do nothing */
      }
      else
      {
         temp->prev->next = temp->next;
         this->queue_ptr->rear_ptr->next = temp;
         temp->prev = this->queue_ptr->rear_ptr;
         temp->next = NULL;
         this->queue_ptr->rear_ptr = temp;
      }
   }
}

int main()
{
   int mcapacity = 5;
   Queue mqueue = {NULL, NULL, mcapacity, 0, get_new_node, enqueue, dequeue, print_queue};
   Lru_cache mcache = {mcapacity,0, &mqueue, NULL, init_cache, refer_page};

   mcache.init_cache_func(&mcache);
   mcache.refer_page_func(&mcache, 0);
   mcache.refer_page_func(&mcache, 1);
   mcache.refer_page_func(&mcache, 2);
   mcache.refer_page_func(&mcache, 3);
   mcache.refer_page_func(&mcache, 4);
   fprintf(stdout,"expected: 0 1 2 3 4\n");
   mqueue.print_queue_func(&mqueue);

   mcache.refer_page_func(&mcache, 3);
   fprintf(stdout,"expected: 0 1 2 4 3\n");
   mqueue.print_queue_func(&mqueue);

   mcache.refer_page_func(&mcache, 1);
   fprintf(stdout,"expected: 0 2 4 3 1\n");
   mqueue.print_queue_func(&mqueue);

   mcache.refer_page_func(&mcache, 0);
   fprintf(stdout,"expected: 2 4 3 1 0\n");
   mqueue.print_queue_func(&mqueue);

   mcache.refer_page_func(&mcache, 5);
}
